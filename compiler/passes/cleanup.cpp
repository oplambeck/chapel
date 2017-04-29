/*
 * Copyright 2004-2017 Cray Inc.
 * Other additional copyright holders may be indicated within.
 *
 * The entirety of this work is licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// cleanup
//
// This pass cleans up the AST after parsing. It handles
// transformations that would be difficult to do while parsing.
//

#include "passes.h"

#include "astutil.h"
#include "build.h"
#include "expr.h"
#include "stlUtil.h"
#include "stmt.h"
#include "stringutil.h"
#include "symbol.h"

static void cleanup(ModuleSymbol* module);

static void normalizeNestedFunctionExpressions(DefExpr* def);

static void flattenScopelessBlock(BlockStmt* block);

static void destructureTupleAssignment(CallExpr* call);

static void flattenPrimaryMethods(FnSymbol* fn);

static void changeCastInWhere(FnSymbol* fn);

static void addParensToDeinitFns(FnSymbol* fn);

void cleanup() {
  std::vector<ModuleSymbol*> mods;

  ModuleSymbol::getTopLevelModules(mods);

  for_vector(ModuleSymbol, mod, mods) {
    cleanup(mod);
  }
}

/************************************* | **************************************
*                                                                             *
*                                                                             *
*                                                                             *
************************************** | *************************************/

static void cleanup(ModuleSymbol* module) {
  std::vector<BaseAST*> asts;

  collect_asts(module, asts);

  for_vector(BaseAST, ast, asts) {
    if (DefExpr* def = toDefExpr(ast)) {
      SET_LINENO(ast);

      normalizeNestedFunctionExpressions(def);
    }
  }

  for_vector(BaseAST, ast, asts) {
    SET_LINENO(ast);

    if (BlockStmt* block = toBlockStmt(ast)) {
      if (block->blockTag == BLOCK_SCOPELESS && block->list != NULL) {
        flattenScopelessBlock(block);
      }

    } else if (CallExpr* call = toCallExpr(ast)) {
      if (call->isNamed("_build_tuple") == true) {
        destructureTupleAssignment(call);
      }

    } else if (DefExpr* def = toDefExpr(ast)) {
      if (FnSymbol* fn = toFnSymbol(def->sym)) {
        flattenPrimaryMethods(fn);
        changeCastInWhere(fn);
        addParensToDeinitFns(fn);
      }
    }
  }
}

/************************************* | **************************************
*                                                                             *
* Moves expressions that are parsed as nested function definitions into their *
* own statement; during parsing, these are embedded in call expressions       *
*                                                                             *
************************************** | *************************************/

static void normalizeNestedFunctionExpressions(DefExpr* def) {
  if (def->sym->hasFlag(FLAG_COMPILER_NESTED_FUNCTION)) {
    Expr* stmt = def->getStmtExpr();

    if (stmt == NULL) {
      if (TypeSymbol* ts = toTypeSymbol(def->parentSymbol)) {
        if (AggregateType* ct = toAggregateType(ts->type)) {
          def->replace(new UnresolvedSymExpr(def->sym->name));

          ct->addDeclarations(def);

          return;
        }
      }
    }

    def->replace(new UnresolvedSymExpr(def->sym->name));
    stmt->insertBefore(def);

  } else if (strncmp("_iterator_for_loopexpr", def->sym->name, 22) == 0) {
    FnSymbol* parent = toFnSymbol(def->parentSymbol);

    INT_ASSERT(strncmp("_parloopexpr", parent->name, 12) == 0 ||
               strncmp("_seqloopexpr", parent->name, 12) == 0);

    while (!strncmp("_parloopexpr", parent->defPoint->parentSymbol->name, 12) ||
           !strncmp("_seqloopexpr", parent->defPoint->parentSymbol->name, 12)) {
      parent = toFnSymbol(parent->defPoint->parentSymbol);
    }

    if (TypeSymbol* ts = toTypeSymbol(parent->defPoint->parentSymbol)) {
      AggregateType* ct = toAggregateType(ts->type);

      INT_ASSERT(ct);

      ct->addDeclarations(def->remove());

    } else {
      parent->defPoint->insertBefore(def->remove());
    }
  }
}

/************************************* | **************************************
*                                                                             *
* Move the statements in a block out of the block                             *
*                                                                             *
************************************** | *************************************/

static void flattenScopelessBlock(BlockStmt* block) {
  for_alist(stmt, block->body) {
    stmt->remove();

    block->insertBefore(stmt);
  }

  block->remove();
}

/************************************* | **************************************
*                                                                             *
* destructureTupleAssignment                                                  *
*                                                                             *
*    (i,j) = expr;    ==>    i = expr(1);                                     *
*                            j = expr(2);                                     *
*                                                                             *
* note: handles recursive tuple destructuring, (i,(j,k)) = ...                *
*                                                                             *
************************************** | *************************************/

static void insertDestructureStatements(Expr*     S1,
                                        Expr*     S2,
                                        CallExpr* lhs,
                                        Expr*     rhs);

static void destructureTupleAssignment(CallExpr* call) {
  CallExpr* parent = toCallExpr(call->parentExpr);

  if (parent               != NULL &&
      parent->isNamed("=") == true &&
      parent->get(1)       == call) {
    VarSymbol* rtmp = newTemp();

    rtmp->addFlag(FLAG_EXPR_TEMP);
    rtmp->addFlag(FLAG_MAYBE_TYPE);
    rtmp->addFlag(FLAG_MAYBE_PARAM);

    Expr* S1 = new CallExpr(PRIM_MOVE, rtmp, parent->get(2)->remove());
    Expr* S2 = new CallExpr(PRIM_NOOP);

    call->getStmtExpr()->replace(S1);

    S1->insertAfter(S2);
    S1->insertBefore(new DefExpr(rtmp));

    insertDestructureStatements(S1, S2, call, new SymExpr(rtmp));

    S2->remove();
  }
}

static void insertDestructureStatements(Expr*     S1,
                                        Expr*     S2,
                                        CallExpr* lhs,
                                        Expr*     rhs) {
  int i = 0;

  S1->getStmtExpr()->insertAfter(
    buildIfStmt(new CallExpr("!=",
                             new SymExpr(new_IntSymbol(lhs->numActuals())),
                             new CallExpr(".",
                                          rhs->copy(),
                                          new_CStringSymbol("size"))),

                new CallExpr("compilerError",
                             new_StringSymbol("tuple size must match the number of grouped variables"),
                             new_IntSymbol(0))));

  for_actuals(expr, lhs) {
    i++;

    expr->remove();

    if (UnresolvedSymExpr* se = toUnresolvedSymExpr(expr)) {
      if (strcmp(se->unresolved, "chpl__tuple_blank") == 0) {
        continue;
      }
    }

    CallExpr* nextLHS = toCallExpr(expr);
    Expr*     nextRHS = new CallExpr(rhs->copy(), new_IntSymbol(i));

    if (nextLHS != NULL && nextLHS->isNamed("_build_tuple") == true) {
      insertDestructureStatements(S1, S2, nextLHS, nextRHS);

    } else {
      VarSymbol* ltmp = newTemp();

      ltmp->addFlag(FLAG_MAYBE_PARAM);

      S1->insertBefore(new DefExpr(ltmp));

      S1->insertBefore(new CallExpr(PRIM_MOVE,
                                    ltmp,
                                    new CallExpr(PRIM_ADDR_OF, expr)));

      S2->insertBefore(new CallExpr("=", ltmp, nextRHS));
    }
  }
}

/************************************* | **************************************
*                                                                             *
*                                                                             *
*                                                                             *
************************************** | *************************************/

static void flattenPrimaryMethods(FnSymbol* fn) {
  if (TypeSymbol* ts = toTypeSymbol(fn->defPoint->parentSymbol)) {
    Expr* insertPoint = ts->defPoint;

    while (isTypeSymbol(insertPoint->parentSymbol)) {
      insertPoint = insertPoint->parentSymbol->defPoint;
    }

    DefExpr* def = fn->defPoint;

    def->remove();

    insertPoint->insertBefore(def);

    if (fn->userString != NULL && fn->name != ts->name) {
      if (strncmp(fn->userString, "ref ", 4) == 0) {
        // fn->userString of "ref foo()"
        // Move "ref " before the type name so we end up with "ref Type.foo()"
        // instead of "Type.ref foo()"
        fn->userString = astr("ref ", ts->name, ".", fn->userString + 4);

      } else {
        fn->userString = astr(ts->name, ".", fn->userString);
      }
    }

    if (ts->hasFlag(FLAG_ATOMIC_TYPE)) {
      fn->addFlag(FLAG_ATOMIC_TYPE);
    }
  }
}

/************************************* | **************************************
*                                                                             *
*                                                                             *
*                                                                             *
************************************** | *************************************/

static void changeCastInWhere(FnSymbol* fn) {
  if (fn->where != NULL) {
    std::vector<BaseAST*> asts;

    collect_asts(fn->where, asts);

    for_vector(BaseAST, ast, asts) {
      if (CallExpr* call = toCallExpr(ast)) {
        if (call->isCast() == true) {
          CallExpr* isSubtype = NULL;
          Expr*     to        = call->castTo();
          Expr*     from      = call->castFrom();

          // now remove to and from so we can add them
          // again as arguments. Don't interleave the
          // remove with the calls to castTo and castFrom.

          to->remove();

          from->remove();

          isSubtype = new CallExpr(PRIM_IS_SUBTYPE, to, from);

          call->replace(isSubtype);
        }
      }
    }
  }
}

/************************************* | **************************************
*                                                                             *
* Make paren-less decls act as paren-ful.                                     *
* Otherwise "arg.deinit()" in proc chpl__delete(arg) would not resolve.       *
*                                                                             *
************************************** | *************************************/

static void addParensToDeinitFns(FnSymbol* fn) {
  if (fn->hasFlag(FLAG_DESTRUCTOR)) {
    fn->removeFlag(FLAG_NO_PARENS);
  }
}

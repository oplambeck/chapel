//===- PDBSymbolTypeVTable.h - VTable type info -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_PDB_PDBSYMBOLTYPEVTABLE_H
#define LLVM_DEBUGINFO_PDB_PDBSYMBOLTYPEVTABLE_H

#include "PDBSymbol.h"
#include "PDBTypes.h"

namespace llvm {

class raw_ostream;
namespace pdb {

class PDBSymbolTypeVTable : public PDBSymbol {
  DECLARE_PDB_SYMBOL_CONCRETE_TYPE(PDB_SymType::VTable)
public:
  void dump(PDBSymDumper &Dumper) const override;

  FORWARD_SYMBOL_ID_METHOD(getClassParent)
  FORWARD_SYMBOL_METHOD(getOffset)
  FORWARD_SYMBOL_METHOD(isConstType)
  FORWARD_SYMBOL_ID_METHOD(getLexicalParent)
  FORWARD_SYMBOL_ID_METHOD(getType)
  FORWARD_SYMBOL_METHOD(isUnalignedType)
  FORWARD_SYMBOL_METHOD(isVolatileType)
};

} // namespace llvm
}

#endif // LLVM_DEBUGINFO_PDB_PDBSYMBOLTYPEVTABLE_H

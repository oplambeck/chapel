use MasonUtils;
use Spawn;
use FileSystem;
use TOML;
use MasonEnv;
use MasonNew;
use MasonModify;
use Random;
use Regexp;

proc masonMortar(args) throws {
  var isPersonal = false;
  var path = '';
  var trueIfLocal = false;

  try! {
    if hasOptions(args, "-h", "--help") {
      masonMortarHelp();
      exit(0);
    }

    const packageHome = getProjectHome(here.cwd());

    if args.size == 2 {
      mortar(MASON_HOME, packageHome, false);
    }
    for arg in args {
      path = arg;
    }
    trueIfLocal = !isPathRemote(path);
    if checkPath(path, trueIfLocal) {
      mortar(path, packageHome, trueIfLocal);
    }
  }
  catch e : MasonError {
    writeln(e.message());
  }
}

proc mortar(path : string, projectHome : string, trueIfLocal : bool) throws {
  var gitResults = gitChecks(path, projectHome, trueIfLocal);
  var moduleResult = moduleCheck(projectHome);
  writeln(gitResults);
  writeln(moduleResult);
  exit(0);
}

private proc moduleCheck(projectHome : string) {
  const subModules = listdir(projectHome + '/src');
  if subModules.size > 1 then return false;
  else return true;
}


private proc gitChecks(path : string , projectHome : string, trueIfLocal : bool) {
  var remoteUrlCheck = gitUrlCheck(projectHome, trueIfLocal);
  return remoteUrlCheck;
}

private proc gitUrlCheck(projectHome : string, trueIfLocal : bool) {
  if !trueIfLocal {
    var result = runCommand('git config --get remote.origin.url', true);
    var status = runWithStatus('git config --get remote.origin.url', false);
    if status != 0 {
      return false;
    }
    writeln(result);
    return true;
  }
  else return true;
}

proc checkPath(path : string, trueIfLocal : bool) throws {
  try! {
    if trueIfLocal {
      if exists(path) && exists(path + "/mason-registry/Bricks/") {
        return true;
      }
      else {
        throw new owned MasonError(path + " is not a valid path");
        exit(0);
      }
    }
    else {
      var command = ('git ls-remote ' + path).split();
      var checkRemote = spawn(command, stdout=CLOSE);
      checkRemote.wait();
      if checkRemote.exit_status == 0 then return true;
      else {
        throw new owned MasonError(path + " is not a valid remote path");
        exit(0);
      }
    }
  }
  catch e : MasonError {
    writeln(e.message());
    exit(0);
  }
}

proc isPathRemote(path : string) throws {
  if path.find(":") == 0 {
    return false;
  }
  else return true;
}

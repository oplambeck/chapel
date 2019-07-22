use MasonUtils;
use Spawn;
use FileSystem;
use TOML;
use MasonEnv;
use MasonNew;
use MasonModify;
use Random;
use Regexp;

proc masonDoctor(args) throws {
  var isPersonal = false;
  var path = '';
  var trueIfLocal = true;

  try! {
    if hasOptions(args, "-h", "--help") {
      masonDoctorHelp();
      exit(0);
    }

    const packageHome = getProjectHome(here.cwd());

    if args.size == 2 {
      doctor(MASON_HOME, packageHome);
    }
    for arg in args {
      if arg == '--registry' {
        isPersonal = true;
      }
      else {
        if isPersonal {
          path = arg;
        }
      }
    }
    doctor(path, packageHome);

  }
  catch e : MasonError {
    writeln(e.message());
  }
}

proc doctor(path : string, projectHome : string) throws {
  var trueIfLocal : bool = true;
  if isPathRemote(path) then trueIfLocal = false;

  checkPath(path, trueIfLocal);
  exit(0);
}

proc checkPath(path : string, trueIfLocal : bool) throws {
  try! {
    if trueIfLocal {
      if exists(path) then return true;
      else {
        throw new owned MasonError(path + " is not a valid path");
        exit(0);
      }
    }
    else {
      var command = ('git ls-remote ' + path).split();
      var checkRemote = spawn(command, stdout=PIPE);
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

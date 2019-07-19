
use MasonUtils;
use Spawn;
use FileSystem;
use TOML;
use MasonEnv;
use MasonNew;
use MasonModify;
use Random;

proc masonDoctor(args) throws {
  var isPersonal = false;
  var path = '';
  var trueIfLocal = true;

  try! {
    if hasOptions(args, "-h", "--help") {
      masonDoctorHelp();
      exit(0);
    }

    const projectHome = getProjectHome(here.cwd());
    for arg in args[2..] {
      if arg == '--registry' {
        isPersonal = true;
      }
      else {
        trueIfLocal = pathCheck(arg);
        path = arg;
      }
    }
    if isPersonal {
      doctor(path, projectHome, trueIfLocal);
    }
    else if args.size == 2 && !isPersonal {
      doctor(MASON_HOME, projectHome, trueIfLocal);
    }
    else {
      throw new owned MasonError('Not a valid Command, see mason doctor help');
    }
  }
  catch e : MasonError {
    writeln(e.message());
    exit(0);
  }

}

proc doctor(path : string, projectHome : string, trueIfLocal : bool) throws {
  try! {
    const openToml = openreader("Mason.toml");
    const tomlFile = parseToml(openToml);

  }
}

proc pathCheck(path : string) throws {
  var isPathLocal = false;

  try! {
    var remoteCheck = spawn(['git ls-remote ' + path], stdout=CLOSE);
    var localCheck = spawn.(['cd ' + path], stdout=CLOSE);
    remoteCheck.wait();
    localCheck.wait();
    if remoteCheck.exit_status  == 0 {
      isPathLocal = false;
      return isPathLocal;
    }
    else if localCheck.exit_status == 0 {
      isPathLocal = true;
      return isPathLocal;
    }
    else {
      throw new owned MasonError(path + ' is not a valid path to a mason-registry');
      exit(0);
    }
  }
  catch e : MasonError {
    writeln(e.message());
    exit(1);
  }
}


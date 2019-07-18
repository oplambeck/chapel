use MasonUtils;
use FileSystem;
use Spawn;
use TOML;


proc MasonDoctor(args) throws {
  var isPersonal = false;
  var path = '';

  try! {
    const projectHome = getProjectHome(here.cwd());
    for arg in args[2..] {
      if arg == '-h' || '--help' {
        masonDoctorHelp();
        exit(0);
      }
      else if arg == '--registry' {
        isPersonal = true;
      }
      else if pathCheck(arg) {
        path = arg;
      }
      else if {
               if !isPersonal {
                               doctor(

               }
        doctor(isLocal, path, projectHome);
      }
    }
  }
  catch {
  }

}


proc pathCheck(path : string) throws {
  var isPathLocal = false;
  try! {
    const remoteCheck = spawn(["runWithStatus('git ls-remote ' + path)"], stdout=CLOSE);
    const localCheck = spawn(["runWithStatus('cd ' + path)"], stdout=CLOSE);
    remoteCheck.wait();
    localCheck.wait();
    if remoteCheck == 0 {
      isPathLocal = false;
      return isPathLocal;
    }
    else if localCheck == 0 {
      isPathLocal = true;
      return isPathLocal;
    }
    else {
      throw new owned MasonError(path + ' is not a valid path to a mason-registry');
    }
  }
  catch e: MasonError{
    writeln(e.message);
    exit(1);
  }
}
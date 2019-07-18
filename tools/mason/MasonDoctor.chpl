
use MasonUtils;
use FileSystem;
use Spawn;
use TOML;


proc MasonDoctor(args) throws {
  var isLocal = false;
  var path = '';

  try! {
    const projectHome = getProjectHome(here.cwd());
    for arg in args[2..] {
      if arg == '-h' || '--help' {
        masonDoctorHelp();
        exit(0);
      }
      else if arg == '--registry' {
        isLocal = true;
      }
      else if pathCheck(arg) {
        path = arg;
      }
      else if {
        doctor(isLocal, path, projectHome);
      }
    }
  }
  catch {
  }

}


proc pathCheck(path : string) throws {


}
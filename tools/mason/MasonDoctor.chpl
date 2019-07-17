
use MasonUtils;
use FileSystem;
use Spawn;
use TOML;


proc MasonDoctor(args) throws {

  try! {
    for arg in args[2..] {
      if arg == '-h' || '--help' {
        masonDoctorHelp();
        exit(0);
      }
      else if arg == '--registry' {
      }{

    }
  }

}
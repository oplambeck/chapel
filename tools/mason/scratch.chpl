
use MasonUtils;
use Spawn;
use FileSystem;
use TOML;
use MasonEnv;
use MasonNew;
use MasonModify;
use Random;




proc masonPublish(args : [] string) throws {
  var dry = false;
  var registry = false;
  var path = MASON_HOME;

  try! {
    if hasOptions(args, "-h", "--help") {
      masonPublishHelp();
      exit(0);
    }
    for arg in args [1..] {
      if arg == "--dry-run" then dry = true;
      else if arg == "--registry" them registry == true;
      else then path = arg;
    }
    
  }
}
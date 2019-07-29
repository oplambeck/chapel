use MasonNew;
use MasonModify;
use MasonUtils;
use MasonSearch;
use TOML;
use FileSystem;

extern proc setenv(name : c_string, envval : c_string, overwrite : c_int) : c_int;

proc main(args: [] string) {

  // Expect basename as first arg, e.g. addDep
  const basename = args[1];

  // Wipe a pre-existing project
  if exists(basename) then rmTree(basename);

  // Create new mason project directory
  mkdir(basename);

  // Replace manifest file
  const tomlFile = basename + '.toml';
  const manifestFile = basename + '/Mason.toml';
  FileSystem.copyFile(tomlFile, manifestFile);

  // here.chdir is not sufficient
  const oldPWD = getEnv('PWD');
  const newPWD = oldPWD + '/' + basename;
  setenv("PWD", newPWD.c_str(), 1);

  // Add mason dependency
  var modArgs = args[2..];
  modArgs.push_front(args[0]);
  masonModify(modArgs);;

  // Print manifest for diff against .good file
  showToml(manifestFile);

  // Cleanup project
  rmTree(basename);
}

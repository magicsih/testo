# Testo

*Testo* is a script-based testing automation application for android games. 

## How to build

### Cmake build (Windows tested only, may not work mac and linux yet)

#### Required libraries

- Boost 1.69.0 (https://www.boost.org/)
- OpenCV 4.0.0 (https://opencv.org/)
- V8(javascript engine) (https://v8.dev/)
- spdlog (https://github.com/gabime/spdlog)

## How to execute

### Configuration

#### Base javascript template

A base javascript file contains some meaningful predefined javascript semantics for various test scenarios. A executable testo file always read *testo.js* once at runtime when it initializes. Remember *testo.js* file is required. Put it in to sample folder with testo.exe.

testo.js

```javascript
function testcase(name, testcaseFunction) {
  print("javascript - Start TestCase : " + name);

  testcaseFunction();

  print("javascript - Finish TestCase : " + name);
}

function Image(name, filePath) {
  return {
    name: name, filePath: filePath
  }
}
```

#### Basic usage

### Allowed options

```bash
  -h [ --help ]         this help message
  -s [ --script ] arg   path to your script file
  -d [ --device ] arg   an android device id (to get an id, use adb.exe command
                        like this <adb.exe devices>
  -a [ --adb ] arg      <optional> path to adb.exe (including directory and
                        executable name. eg. /usr/local/bin/adb,
                        C:/Android/nox_adb.exe). if it's not specified, this
                        program will try to run adb.exe in current directory
                        passion. (it can be pointing to path for customized adb
                        like nox_adb
  -k [ --apk ] arg      <optional> path to apk file. if it's specified, this
                        program will try to install and run it
  -p [ --package ] arg  <optional> package name to start before script
                        processing
  --debug               <optional> print logs in debug verbosity
```

Default

```bash
testo -s yourscript.js -a "{full path to adb.exe}" -d {device_id} -p {package_name_to_start_with}"
```

If you want to use Nox emulator instead of real mobile device
```bash
testo -s yourscript.js -a "C:/Program Files/Nox/bin/nox_adb.exe" -d 127.0.0.1:62001 -p com.etlegame.reversi
```

yourscript.js means really a script you written.

#### Your Script Example

```javascript
testcase('first test', function() {
    waitUntilCondition(function(){ 
        return (matchImage(new Image('title', 'http://imagelocation'))); 
    }, 60 );

    tapImage(new Image('single player', 'http://imagelocation'));
});

testcase('just swipe test', function() {
    swipe(100,100,800,100);
    swipe(800,100,100,100);
});
```



## Supported Script API

- tap(x,y) - tap x,y of the screen
- swipe(x1,y1,x2,y2) - swipe from x1,y1 to x2,y2
- tapImage(image, matchingRate) : find image from a screenshot. then tap the center of the screenshot.
- waitUntilCondition(condition) : wait until condition turns to be true. condition can be defined as a function.

## TODO

- Scripting support tool. (screenshot, axis) (We have web-based scripting tool already)
- Supporting android UI automator features.
- Make possible to use Unity and Unreal Game engine object. (We have SDK already)
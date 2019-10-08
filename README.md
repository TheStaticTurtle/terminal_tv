# Terminal TV

Terminal TV is a video player that allows you to display a video (or camera) into your terminal using ansi color codes. A full screen console with a font size of 11 can do approximately 30-35 fps

## Documentation
### Building

Libs requiered: libopencv-dev

    samuel@FlutterShy:~/terminal_tv$ mkdir build
    samuel@FlutterShy:~/terminal_tv$ cd build
    samuel@FlutterShy:~/terminal_tv/build$ cmake ..
    samuel@FlutterShy:~/terminal_tv/build$ make

### Usage

You can use: 

    ./terminal_tv input.mp4 

to load a mp4 video or this:

    ./terminal_tv /dev/video0

to load a camera. You can pretty much put any path that opencv accept and it will work. 
To remove all the information at the top by redirection the stderr to /dev/null:

    ./terminal_tv input.mp4 2>/dev/null
    
### Demo
![Example](https://i.imgur.com/fieQaZI.png)[Video demonstration (https://www.youtube.com/watch?v=YMIr55X8WbQ) ](https://www.youtube.com/watch?v=YMIr55X8WbQ)


<img src="runcat.gif" width="150" align="left" />

# runcat-tray

Is a runcat port for Linux using libappindicator.

icons from: https://github.com/win0err/gnome-runcat

## requirements

- libappindicator-gtk3

## build

> mkdir build && cd build
>
> cmake .. && make
>
> make install

It will install all default icons to your `~/.config/runcat/icons`, optionally, you can copy the runcat binary to your $PATH.

## start

> runcat


there're several flags can use

- `-l` to set the lower bound of FPS, default to 6
- `-u` to set the upper bound of FPS, default to 90
- `-d` to set the default icon set, default to $HOME/.config/runcat/icons/cat 

## default resources

TODO:

<img src="runcat.gif" width="150" align="left" />

# runcat-tray

Is a runcat port for Linux using libappindicator.

(Another useless cat here..)

## requirements

- libappindicator-gtk3

## build

> mkdir build && cd build
>
> cmake .. && make
>
> make install

It will install all default icons to your `~/.config/runcat/icons`, optionally, you can move/copy the runcat binary to your $PATH.

## start

> runcat


there're several flags can use

- `-l` to set the lower bound of FPS, default to 6
- `-u` to set the upper bound of FPS, default to 90
- `-d` to set the default icon set, default to $HOME/.config/runcat/icons/cat 

## resources

The default resources are from internet.

- <img src="runcat.gif" width="36" /> from [win0err/gnome-runcat](https://github.com/win0err/gnome-runcat)
- <img src="https://c.tenor.com/5IWFYb4D1WMAAAAi/swan_hack-dab.gif" width="36" /> from [unreturnable](https://tenor.com/view/swan_hack-dab-tux-linux-gif-19010779)
- <img src="https://github.githubassets.com/images/mona-loading-default.gif" width="36" /> from github home page
- <img src="https://cultofthepartyparrot.com/guests/hd/partyblobcat.gif" width="36" /> from [cultofthepartyparrot.com](https://cultofthepartyparrot.com/)

You can use your custom pictures.

1. extract all frames(png/svg are good) from a gif, usually using `imagemagick`'s `convert` command, like

> convert -coalesce ./input.gif out%02d.png

2. copy those frames in a directory.
3. use the `-d` flag to specify the icons' ABS path(like `/home/username/.config/runcat/icons/xxx`).

PS: It currently support no more than 30 frames in a directory.

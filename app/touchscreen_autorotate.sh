#!/bin/bash

TRANSFORM='Coordinate Transformation Matrix'
TOUCHPAD=$(xinput list --name-only | grep Touchpad | head -n1)
SCR=$(xrandr --current | grep primary | sed -e 's/ .*//g');

monitor-sensor | while read -r line;
do
    if [[ $line =~ .*left.* ]]
    then
        xinput set-prop "$TOUCHPAD" "$TRANSFORM" 0 -1 1 1 0 0 0 0 1
        xrandr --output $SCR --rotate left;
    fi

    if [[ $line =~ .*right* ]]
    then
        xinput set-prop "$TOUCHPAD" "$TRANSFORM" 0 1 0 -1 0 1 0 0 1
        xrandr --output $SCR --rotate right;
    fi

    if [[ $line =~ .*bottom-up.* ]]
    then
        xinput set-prop "$TOUCHPAD" "$TRANSFORM" -1 0 1 0 -1 1 0 0 1
        xrandr --output $SCR --rotate inverted;
    fi

    if [[ $line =~ .*normal.* ]]
    then
        xinput set-prop "$TOUCHPAD" "$TRANSFORM" 1 0 0 0 1 0 0 0 1
        xrandr --output $SCR --rotate normal;
    fi
done

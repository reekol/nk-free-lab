touch /entrypoint.log

DISPLAY=${NOVNC_DISPLAY}

screen -S nk_novncapp -dm \
    xvfb-run --listen-tcp --server-num $DISPLAY --auth-file /tmp/xvfb.auth \
    -s "-ac -screen 0 ${NOVNC_WIDTH}x${NOVNC_HEIGHT}x24" \
    /usr/bin/kate
#   /usr/share/code/bin/code --no-sandbox --user-data-dir /code

sleep 3
APP_ID=$(export DISPLAY=:$DISPLAY && xdotool search --onlyvisible kate)

$(export DISPLAY=:$DISPLAY && xdotool windowmove $APP_ID 0 0)
$(export DISPLAY=:$DISPLAY && xdotool windowsize $APP_ID ${NOVNC_WIDTH} ${NOVNC_HEIGHT})

x11vnc -storepasswd ${NOVNC_PASSWORD} /tmp/vncpass
screen -S nk_vnc -dm \
    x11vnc  -shared -rfbport 5901 -rfbauth /tmp/vncpass -display :${DISPLAY} -forever -auth /tmp/xvfb.auth

screen -ls
websockify --web /usr/share/novnc 80 localhost:5901

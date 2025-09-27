#! /bin/bash

$TERM --command "/home/eggbert/programs/c/video_call/bin/server" "localhost" "25565" &
$TERM --command "/home/eggbert/programs/c/video_call/bin/client" "localhost" "25565"

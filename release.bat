@echo off
mkdir game
copy ld41 game
xcopy data game\data /e /i /h
mkdir game\s
echo ld41 > game\s\startup-sequence
exe2adf ld41 -l "ld41-unk" -a "ld41-unk.adf" -d game
rmdir game /s /q

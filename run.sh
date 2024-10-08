#!/bin/bash

export MAP_PATH=/mnt/hdd_2/map_cache
#----- Index create ---------------------------------------------------------------------
./MBTilesArchive -ci 0,8,0,* -i mbt_z0-8.index -p $MAP_PATH
./MBTilesArchive -ci 9,9,0,255 -i mbt_z9x0-255.index -p $MAP_PATH
./MBTilesArchive -ci 9,9,256,511 -i mbt_z9x256-511.index -p $MAP_PATH
./MBTilesArchive -ci 10,10,0,255 -i mbt_z10x0-255.index -p $MAP_PATH
./MBTilesArchive -ci 10,10,256,511 -i mbt_z10x256-511.index -p $MAP_PATH
./MBTilesArchive -ci 10,10,512,767 -i mbt_z10x512-767.index -p $MAP_PATH
./MBTilesArchive -ci 10,10,768,1023 -i mbt_z10x768-1023.index -p $MAP_PATH

./MBTilesArchive -ci 11,11,0,127 -i mbt_z11x0-127.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,128,255 -i mbt_z11x128-255.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,256,383 -i mbt_z11x256-383.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,384,511 -i mbt_z11x384-511.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,512,639 -i mbt_z11x512-639.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,640,767 -i mbt_z11x640-767.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,768,895 -i mbt_z11x768-895.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,896,1023 -i mbt_z11x896-1023.index -p $MAP_PATH

./MBTilesArchive -ci 11,11,1024,1151 -i mbt_z11x1024-1151.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,1152,1279 -i mbt_z11x1152-1279.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,1280,1407 -i mbt_z11x1280-1407.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,1408,1535 -i mbt_z11x1408-1535.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,1536,1663 -i mbt_z11x1536-1663.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,1664,1791 -i mbt_z11x1664-1791.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,1792,1919 -i mbt_z11x1792-1919.index -p $MAP_PATH
./MBTilesArchive -ci 11,11,1920,2047 -i mbt_z11x1920-2047.index -p $MAP_PATH

#----- SQLite create ---------------------------------------------------------------------
./MBTilesArchive -p $MAP_PATH -i mbt_z0-8.index -d mbt_z0-8.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z9x0-255.index -d mbt_z9x0-255.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z9x256-511.index -d mbt_z9x256-511.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z10x0-255.index -d mbt_z10x0-255.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z10x256-511.index -d mbt_z10x256-511.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z10x512-767.index -d mbt_z10x512-767.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z10x768-1023.index -d mbt_z10x768-1023.sqlite

./MBTilesArchive -p $MAP_PATH -i mbt_z11x0-127.index -d mbt_z11x0-127.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x128-255.index -d mbt_z11x128-255.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x256-383.index -d mbt_z11x256-383.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x384-511.index -d mbt_z11x384-511.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x512-639.index -d mbt_z11x512-639.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x640-767.index -d mbt_z11x640-767.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x768-895.index -d mbt_z11x768-895.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x896-1023.index -d mbt_z11x896-1023.sqlite

./MBTilesArchive -p $MAP_PATH -i mbt_z11x1024-1151.index -d mbt_z11x1024-1151.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x1152-1279.index -d mbt_z11x1152-1279.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x1280-1407.index -d mbt_z11x1280-1407.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x1408-1535.index -d mbt_z11x1408-1535.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x1536-1663.index -d mbt_z11x1536-1663.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x1664-1791.index -d mbt_z11x1664-1791.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x1792-1919.index -d mbt_z11x1792-1919.sqlite
./MBTilesArchive -p $MAP_PATH -i mbt_z11x1920-2047.index -d mbt_z11x1920-2047.sqlite

[----------------- upsample_bilinear2d channels_first contiguous -----------------]
                                       |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |          314.8       |          332.2     
      [1, 3, 320, 320] -> (512, 512)   |         1179.6       |         1253.2     
      [32, 128, 64, 64] -> (32, 32)    |         9756.6       |        10322.9     
      [32, 128, 64, 64] -> (128, 128)  |       202922.0       |       206999.4     
      [1, 3, 500, 500] -> (256, 256)   |          324.7       |          343.1     
      [1, 3, 500, 500] -> (800, 800)   |         2843.8       |         3025.6     
6 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |           73.0       |           75.8     
      [1, 3, 320, 320] -> (512, 512)   |          231.7       |          245.4     
      [32, 128, 64, 64] -> (32, 32)    |         2303.7       |         2323.9     
      [32, 128, 64, 64] -> (128, 128)  |        51782.8       |        53141.2     
      [1, 3, 500, 500] -> (256, 256)   |           74.5       |           78.9     
      [1, 3, 500, 500] -> (800, 800)   |          535.9       |          567.7     

Times are in microseconds (us).

[-------------- upsample_bilinear2d channels_first non-contiguous ---------------]
                                      |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: -----------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)  |         318.1        |         336.1      
      [1, 3, 320, 320] -> (512, 512)  |        1184.1        |        1256.6      
      [1, 3, 500, 500] -> (256, 256)  |         329.4        |         347.0      
      [1, 3, 500, 500] -> (800, 800)  |        2852.3        |        3034.0      
6 threads: -----------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)  |          74.2        |          77.5      
      [1, 3, 320, 320] -> (512, 512)  |         231.3        |         245.7      
      [1, 3, 500, 500] -> (256, 256)  |          75.2        |          79.0      
      [1, 3, 500, 500] -> (800, 800)  |         536.5        |         569.5      

Times are in microseconds (us).

[---------------- upsample_bilinear2d channels_last non-contiguous ---------------]
                                       |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |         1008.8       |         1278.2     
      [1, 3, 320, 320] -> (512, 512)   |         4005.4       |         5058.9     
      [32, 128, 64, 64] -> (32, 32)    |         5907.1       |         6252.0     
      [32, 128, 64, 64] -> (128, 128)  |       109818.4       |       112478.3     
      [2, 128, 64, 46] -> (32, 32)     |          109.9       |          131.2     
      [2, 128, 64, 46] -> (128, 128)   |         1647.9       |         1774.1     
      [1, 3, 500, 500] -> (256, 256)   |         1010.5       |         1287.3     
      [1, 3, 500, 500] -> (800, 800)   |         9784.5       |        12260.9     
6 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |         1090.0       |          258.4     
      [1, 3, 320, 320] -> (512, 512)   |         4235.3       |          974.8     
      [32, 128, 64, 64] -> (32, 32)    |         2266.6       |         2306.2     
      [32, 128, 64, 64] -> (128, 128)  |        36627.7       |        34763.1     
      [2, 128, 64, 46] -> (32, 32)     |           60.4       |           38.5     
      [2, 128, 64, 46] -> (128, 128)   |          900.7       |          570.6     
      [1, 3, 500, 500] -> (256, 256)   |         1092.0       |          258.1     
      [1, 3, 500, 500] -> (800, 800)   |        10103.4       |         2348.6     

Times are in microseconds (us).

[------------- upsample_linear1d channels_first contiguous --------------]
                              |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: ---------------------------------------------------------------
      [4, 512, 320] -> [256]  |        515.9         |         518.3      
      [4, 512, 320] -> [512]  |        997.7         |        1008.5      
6 threads: ---------------------------------------------------------------
      [4, 512, 320] -> [256]  |        103.9         |         104.6      
      [4, 512, 320] -> [512]  |        192.0         |         194.2      

Times are in microseconds (us).

[-------------------- upsample_trilinear3d channels_first contiguous --------------------]
                                              |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: -------------------------------------------------------------------------------
      [1, 3, 16, 320, 320] -> [8, 256, 256]   |         11.4         |         11.3       
      [1, 3, 16, 320, 320] -> [32, 512, 512]  |        210.6         |        211.9       
6 threads: -------------------------------------------------------------------------------
      [1, 3, 16, 320, 320] -> [8, 256, 256]   |          2.1         |          2.1       
      [1, 3, 16, 320, 320] -> [32, 512, 512]  |         43.4         |         42.8       

Times are in milliseconds (ms).

[------------------ upsample_trilinear3d channels_last non-contiguous -------------------]
                                              |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: -------------------------------------------------------------------------------
      [1, 3, 16, 320, 320] -> [8, 256, 256]   |         13.8         |         20.2       
      [1, 3, 16, 320, 320] -> [32, 512, 512]  |        253.7         |        354.8       
6 threads: -------------------------------------------------------------------------------
      [1, 3, 16, 320, 320] -> [8, 256, 256]   |         14.1         |          3.6       
      [1, 3, 16, 320, 320] -> [32, 512, 512]  |        254.6         |         66.9       

Times are in milliseconds (ms).

[------------------ upsample_nearest2d channels_first contiguous -----------------]
                                       |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |          747.3       |          370.5     
      [1, 3, 320, 320] -> (512, 512)   |         2963.3       |         1442.3     
      [32, 128, 64, 64] -> (32, 32)    |        16322.1       |         8552.6     
      [32, 128, 64, 64] -> (128, 128)  |       199322.3       |       217408.7     
      [1, 3, 500, 500] -> (256, 256)   |          752.8       |          374.3     
      [1, 3, 500, 500] -> (800, 800)   |         7223.5       |         3498.1     
6 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |          139.3       |           78.5     
      [1, 3, 320, 320] -> (512, 512)   |          538.8       |          270.3     
      [32, 128, 64, 64] -> (32, 32)    |         3026.5       |         2243.4     
      [32, 128, 64, 64] -> (128, 128)  |        51159.8       |        54083.6     
      [1, 3, 500, 500] -> (256, 256)   |          140.4       |           79.1     
      [1, 3, 500, 500] -> (800, 800)   |         1308.1       |          643.0     

Times are in microseconds (us).

[--------------- upsample_nearest2d channels_first non-contiguous ---------------]
                                      |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: -----------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)  |         831.5        |         371.3      
      [1, 3, 320, 320] -> (512, 512)  |        3054.7        |        1442.1      
      [1, 3, 500, 500] -> (256, 256)  |        1026.8        |         375.4      
      [1, 3, 500, 500] -> (800, 800)  |        7563.4        |        3499.4      
6 threads: -----------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)  |         158.5        |          78.5      
      [1, 3, 320, 320] -> (512, 512)  |         559.7        |         272.4      
      [1, 3, 500, 500] -> (256, 256)  |         193.5        |          79.1      
      [1, 3, 500, 500] -> (800, 800)  |        1384.0        |         644.6      

Times are in microseconds (us).

[---------------- upsample_nearest2d channels_last non-contiguous ----------------]
                                       |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |         691.0        |          479.2     
      [1, 3, 320, 320] -> (512, 512)   |        2740.1        |         1845.7     
      [32, 128, 64, 64] -> (32, 32)    |        3295.6        |         3415.9     
      [32, 128, 64, 64] -> (128, 128)  |       99946.8        |       101278.1     
      [2, 128, 64, 46] -> (32, 32)     |          73.9        |           85.2     
      [2, 128, 64, 46] -> (128, 128)   |        1568.3        |         1678.3     
      [1, 3, 500, 500] -> (256, 256)   |         694.7        |          487.6     
      [1, 3, 500, 500] -> (800, 800)   |        6688.9        |         4489.5     
6 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |         129.7        |          100.0     
      [1, 3, 320, 320] -> (512, 512)   |         499.1        |          347.4     
      [32, 128, 64, 64] -> (32, 32)    |        1387.1        |         1414.1     
      [32, 128, 64, 64] -> (128, 128)  |       30951.6        |        31586.9     
      [2, 128, 64, 46] -> (32, 32)     |          17.6        |           26.1     
      [2, 128, 64, 46] -> (128, 128)   |         526.5        |          549.6     
      [1, 3, 500, 500] -> (256, 256)   |         130.6        |          101.7     
      [1, 3, 500, 500] -> (800, 800)   |        1210.5        |          820.9     

Times are in microseconds (us).

[------------- upsample_nearest1d channels_first contiguous -------------]
                              |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: ---------------------------------------------------------------
      [4, 512, 320] -> [256]  |         940.4        |        425.8       
      [4, 512, 320] -> [512]  |        1859.9        |        818.8       
6 threads: ---------------------------------------------------------------
      [4, 512, 320] -> [256]  |         173.7        |         85.6       
      [4, 512, 320] -> [512]  |         339.5        |        156.3       

Times are in microseconds (us).

[--------------------- upsample_nearest3d channels_first contiguous ---------------------]
                                              |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: -------------------------------------------------------------------------------
      [1, 3, 16, 320, 320] -> [8, 256, 256]   |         8215.9       |        3522.2      
      [1, 3, 16, 320, 320] -> [32, 512, 512]  |       128880.1       |       86850.0      
6 threads: -------------------------------------------------------------------------------
      [1, 3, 16, 320, 320] -> [8, 256, 256]   |         1494.6       |         655.1      
      [1, 3, 16, 320, 320] -> [32, 512, 512]  |        27710.2       |       20180.3      

Times are in microseconds (us).

[------------------- upsample_nearest3d channels_last non-contiguous --------------------]
                                              |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: -------------------------------------------------------------------------------
      [1, 3, 16, 320, 320] -> [8, 256, 256]   |         7047.4       |         4958.6     
      [1, 3, 16, 320, 320] -> [32, 512, 512]  |       131452.9       |       110582.9     
6 threads: -------------------------------------------------------------------------------
      [1, 3, 16, 320, 320] -> [8, 256, 256]   |         1280.0       |          863.3     
      [1, 3, 16, 320, 320] -> [32, 512, 512]  |        28504.1       |        24237.4     

Times are in microseconds (us).

[------------------ upsample_bicubic2d channels_first contiguous -----------------]
                                       |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |         6278.5       |         1669.6     
      [1, 3, 320, 320] -> (512, 512)   |        25075.0       |         6541.3     
      [32, 128, 64, 64] -> (32, 32)    |       443528.9       |        38617.8     
      [32, 128, 64, 64] -> (128, 128)  |      7330044.0       |       653620.9     
      [1, 3, 500, 500] -> (256, 256)   |         6290.5       |         1680.4     
      [1, 3, 500, 500] -> (800, 800)   |        61264.9       |        15938.1     
6 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |         7213.7       |          347.9     
      [1, 3, 320, 320] -> (512, 512)   |        25383.0       |         1248.4     
      [32, 128, 64, 64] -> (32, 32)    |       434022.7       |         6831.0     
      [32, 128, 64, 64] -> (128, 128)  |      7054144.1       |       131920.9     
      [1, 3, 500, 500] -> (256, 256)   |         6575.6       |          349.3     
      [1, 3, 500, 500] -> (800, 800)   |        62155.9       |         2967.3     

Times are in microseconds (us).

[--------------- upsample_bicubic2d channels_first non-contiguous ---------------]
                                      |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: -----------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)  |        6368.0        |        1673.8      
      [1, 3, 320, 320] -> (512, 512)  |       25159.9        |        6550.2      
      [1, 3, 500, 500] -> (256, 256)  |        6571.6        |        1682.0      
      [1, 3, 500, 500] -> (800, 800)  |       61627.6        |       15945.1      
6 threads: -----------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)  |        6598.9        |         349.1      
      [1, 3, 320, 320] -> (512, 512)  |       25433.8        |        1249.8      
      [1, 3, 500, 500] -> (256, 256)  |        6690.1        |         349.8      
      [1, 3, 500, 500] -> (800, 800)  |       61664.7        |        2973.6      

Times are in microseconds (us).

[---------------- upsample_bicubic2d channels_last non-contiguous ----------------]
                                       |  1.9.0a0+git6aa5148  |  1.9.0a0+git60fc37d
1 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |         6430.0       |         1674.9     
      [1, 3, 320, 320] -> (512, 512)   |        25220.6       |         6592.1     
      [32, 128, 64, 64] -> (32, 32)    |       511184.8       |        76709.2     
      [32, 128, 64, 64] -> (128, 128)  |      7180465.9       |       854566.1     
      [2, 128, 64, 46] -> (32, 32)     |        10783.9       |         4432.2     
      [2, 128, 64, 46] -> (128, 128)   |       151788.2       |        42934.3     
      [1, 3, 500, 500] -> (256, 256)   |         6654.1       |         1703.8     
      [1, 3, 500, 500] -> (800, 800)   |        61748.1       |        16020.4     
6 threads: ------------------------------------------------------------------------
      [1, 3, 320, 320] -> (256, 256)   |         6627.6       |          349.3     
      [1, 3, 320, 320] -> (512, 512)   |        25472.1       |         1257.2     
      [32, 128, 64, 64] -> (32, 32)    |       485729.0       |        16109.3     
      [32, 128, 64, 64] -> (128, 128)  |      7241317.9       |       170056.8     
      [2, 128, 64, 46] -> (32, 32)     |        10105.0       |          883.2     
      [2, 128, 64, 46] -> (128, 128)   |       150438.8       |         7714.1     
      [1, 3, 500, 500] -> (256, 256)   |         6712.0       |          355.2     
      [1, 3, 500, 500] -> (800, 800)   |        62358.6       |         2982.3     

Times are in microseconds (us).


Intermediate benchmark sources:

- results/custom_pth_nightly_results_1.9.0a0+git6aa5148.log.save
- results/custom_pr_results_1.9.0a0+git60fc37d.log.save

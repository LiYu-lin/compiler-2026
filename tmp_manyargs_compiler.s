	.text
	.globl f
	.type f, @function
f:
	li t0, 3504
	sub sp, sp, t0
Label_1:
	lw v_tmp, 220(sp)
	sw v_tmp, 0(sp)
	lw v_tmp, 252(sp)
	sw v_tmp, 4(sp)
	lw v_tmp, 260(sp)
	sw v_tmp, 8(sp)
	sw v@arg3, 12(sp)
	lw v_tmp, 308(sp)
	sw v_tmp, 16(sp)
	sw v@arg5, 20(sp)
	sw v@arg6, 24(sp)
	sw v@arg7, 28(sp)
	li t6, 3504
	add t6, sp, t6
	lw v_tmp, 0(t6)
	sw v_tmp, 204(sp)
	lw v_tmp, 204(sp)
	sw v_tmp, 32(sp)
	li t6, 3512
	add t6, sp, t6
	lw v_tmp, 0(t6)
	sw v_tmp, 284(sp)
	lw v_tmp, 284(sp)
	sw v_tmp, 36(sp)
	li t6, 3520
	add t6, sp, t6
	lw v_tmp, 0(t6)
	sw v_tmp, 212(sp)
	lw v_tmp, 212(sp)
	sw v_tmp, 40(sp)
	lw v_tmp, 220(sp)
	sw v_tmp, 44(sp)
	lw v_tmp, 252(sp)
	sw v_tmp, 52(sp)
	lw v_tmp, 44(sp)
	sw v_tmp, 292(sp)
	lw v_tmp, 52(sp)
	sw v_tmp, 268(sp)
	lw v_tmp, 292(sp)
	sw v_tmp, 316(sp)
	lw v_tmp, 268(sp)
	sw v_tmp, 356(sp)
	lw v_tmp, 316(sp)
	sw v_tmp, 332(sp)
	lw v_tmp, 332(sp)
	sw v_tmp, 380(sp)
	lw v_tmp, 356(sp)
	sw v_tmp, 372(sp)
	lw v_tmp, 372(sp)
	sw v_tmp, 420(sp)
	lw v_tmp, 380(sp)
	sw v_tmp, 396(sp)
	lw v_tmp, 396(sp)
	sw v_tmp, 428(sp)
	lw v_tmp, 420(sp)
	sw v_tmp, 444(sp)
	lw v_tmp, 428(sp)
	sw v_tmp, 476(sp)
	lw v_tmp, 444(sp)
	sw v_tmp, 468(sp)
	lw v_tmp, 468(sp)
	sw v_tmp, 508(sp)
	lw v_tmp, 476(sp)
	sw v_tmp, 500(sp)
	lw v_tmp, 500(sp)
	sw v_tmp, 524(sp)
	lw v_tmp, 508(sp)
	sw v_tmp, 556(sp)
	lw v_tmp, 524(sp)
	sw v_tmp, 548(sp)
	lw v_tmp, 548(sp)
	sw v_tmp, 580(sp)
	lw v_tmp, 556(sp)
	sw v_tmp, 604(sp)
	lw v_tmp, 580(sp)
	sw v_tmp, 596(sp)
	lw v_tmp, 596(sp)
	sw v_tmp, 628(sp)
	lw v_tmp, 604(sp)
	sw v_tmp, 660(sp)
	lw v_tmp, 628(sp)
	sw v_tmp, 644(sp)
	lw v_tmp, 644(sp)
	sw v_tmp, 692(sp)
	lw v_tmp, 660(sp)
	sw v_tmp, 676(sp)
	lw v_tmp, 676(sp)
	sw v_tmp, 716(sp)
	lw v_tmp, 692(sp)
	sw v_tmp, 708(sp)
	lw v_tmp, 708(sp)
	sw v_tmp, 748(sp)
	lw v_tmp, 716(sp)
	sw v_tmp, 732(sp)
	lw v_tmp, 732(sp)
	sw v_tmp, 772(sp)
	lw v_tmp, 748(sp)
	sw v_tmp, 788(sp)
	lw v_tmp, 772(sp)
	sw v_tmp, 812(sp)
	lw v_tmp, 788(sp)
	sw v_tmp, 796(sp)
	lw v_tmp, 796(sp)
	sw v_tmp, 852(sp)
	lw v_tmp, 812(sp)
	sw v_tmp, 828(sp)
	lw v_tmp, 828(sp)
	sw v_tmp, 876(sp)
	lw v_tmp, 852(sp)
	sw v_tmp, 868(sp)
	lw v_tmp, 868(sp)
	sw v_tmp, 900(sp)
	lw v_tmp, 876(sp)
	sw v_tmp, 924(sp)
	lw v_tmp, 900(sp)
	sw v_tmp, 916(sp)
	lw v_tmp, 916(sp)
	sw v_tmp, 956(sp)
	lw v_tmp, 924(sp)
	sw v_tmp, 948(sp)
	lw v_tmp, 948(sp)
	sw v_tmp, 996(sp)
	lw v_tmp, 956(sp)
	sw v_tmp, 972(sp)
	lw v_tmp, 972(sp)
	sw v_tmp, 1004(sp)
	lw v_tmp, 996(sp)
	sw v_tmp, 1020(sp)
	lw v_tmp, 1004(sp)
	sw v_tmp, 1036(sp)
	lw v_tmp, 1020(sp)
	sw v_tmp, 1076(sp)
	lw v_tmp, 1036(sp)
	sw v_tmp, 1052(sp)
	lw v_tmp, 1052(sp)
	sw v_tmp, 1108(sp)
	lw v_tmp, 1076(sp)
	sw v_tmp, 1084(sp)
	lw v_tmp, 1084(sp)
	sw v_tmp, 1116(sp)
	lw v_tmp, 1108(sp)
	sw v_tmp, 1140(sp)
	lw v_tmp, 1116(sp)
	sw v_tmp, 1164(sp)
	lw v_tmp, 1140(sp)
	sw v_tmp, 1156(sp)
	lw v_tmp, 1156(sp)
	sw v_tmp, 1196(sp)
	lw v_tmp, 1164(sp)
	sw v_tmp, 1188(sp)
	lw v_tmp, 1188(sp)
	sw v_tmp, 1228(sp)
	lw v_tmp, 1196(sp)
	sw v_tmp, 1220(sp)
	lw v_tmp, 1220(sp)
	sw v_tmp, 1244(sp)
	lw v_tmp, 1228(sp)
	sw v_tmp, 1268(sp)
	lw v_tmp, 1244(sp)
	sw v_tmp, 1292(sp)
	lw v_tmp, 1268(sp)
	sw v_tmp, 1284(sp)
	lw v_tmp, 1284(sp)
	sw v_tmp, 1316(sp)
	lw v_tmp, 1292(sp)
	sw v_tmp, 1348(sp)
	lw v_tmp, 1316(sp)
	sw v_tmp, 1324(sp)
	lw v_tmp, 1324(sp)
	sw v_tmp, 1380(sp)
	lw v_tmp, 1348(sp)
	sw v_tmp, 1356(sp)
	lw v_tmp, 1356(sp)
	sw v_tmp, 1412(sp)
	lw v_tmp, 1380(sp)
	sw v_tmp, 1396(sp)
	lw v_tmp, 1396(sp)
	sw v_tmp, 1444(sp)
	lw v_tmp, 1412(sp)
	sw v_tmp, 1428(sp)
	lw v_tmp, 1428(sp)
	sw v_tmp, 1468(sp)
	lw v_tmp, 1444(sp)
	sw v_tmp, 1452(sp)
	lw v_tmp, 1452(sp)
	sw v_tmp, 1484(sp)
	lw v_tmp, 1468(sp)
	sw v_tmp, 1508(sp)
	lw v_tmp, 1484(sp)
	sw v_tmp, 1524(sp)
	lw v_tmp, 1508(sp)
	sw v_tmp, 1532(sp)
	lw v_tmp, 1524(sp)
	sw v_tmp, 1564(sp)
	lw v_tmp, 1532(sp)
	sw v_tmp, 1556(sp)
	lw v_tmp, 1556(sp)
	sw v_tmp, 1596(sp)
	lw v_tmp, 1564(sp)
	sw v_tmp, 1588(sp)
	lw v_tmp, 1588(sp)
	sw v_tmp, 1612(sp)
	lw v_tmp, 1596(sp)
	sw v_tmp, 1628(sp)
	lw v_tmp, 1612(sp)
	sw v_tmp, 1652(sp)
	lw v_tmp, 1628(sp)
	sw v_tmp, 1684(sp)
	lw v_tmp, 1652(sp)
	sw v_tmp, 1660(sp)
	lw v_tmp, 1660(sp)
	sw v_tmp, 1692(sp)
	lw v_tmp, 1684(sp)
	sw v_tmp, 1708(sp)
	lw v_tmp, 1692(sp)
	sw v_tmp, 1732(sp)
	lw v_tmp, 1708(sp)
	sw v_tmp, 1756(sp)
	lw v_tmp, 1732(sp)
	sw v_tmp, 1748(sp)
	lw v_tmp, 1748(sp)
	sw v_tmp, 1796(sp)
	lw v_tmp, 1756(sp)
	sw v_tmp, 1780(sp)
	lw v_tmp, 1780(sp)
	sw v_tmp, 1804(sp)
	lw v_tmp, 1796(sp)
	sw v_tmp, 1820(sp)
	lw v_tmp, 1804(sp)
	sw v_tmp, 1836(sp)
	lw v_tmp, 1820(sp)
	sw v_tmp, 1868(sp)
	lw v_tmp, 1836(sp)
	sw v_tmp, 1852(sp)
	lw v_tmp, 1852(sp)
	sw v_tmp, 1900(sp)
	lw v_tmp, 1868(sp)
	sw v_tmp, 1884(sp)
	lw v_tmp, 1884(sp)
	sw v_tmp, 1932(sp)
	lw v_tmp, 1900(sp)
	sw v_tmp, 1916(sp)
	lw v_tmp, 1916(sp)
	sw v_tmp, 1964(sp)
	lw v_tmp, 1932(sp)
	sw v_tmp, 1956(sp)
	lw v_tmp, 1956(sp)
	sw v_tmp, 1988(sp)
	lw v_tmp, 1964(sp)
	sw v_tmp, 2004(sp)
	lw v_tmp, 1988(sp)
	sw v_tmp, 2012(sp)
	lw v_tmp, 2004(sp)
	li t6, 2052
	add t6, sp, t6
	sw v_tmp, 0(t6)
	lw v_tmp, 2012(sp)
	sw v_tmp, 2036(sp)
	lw v_tmp, 2036(sp)
	li t6, 2060
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2052
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2084
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2060
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2092
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2084
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2132
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2092
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2116
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2116
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2140
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2132
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2180
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2140
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2164
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2164
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2188
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2180
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2212
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2188
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2244
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2212
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2228
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2228
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2252
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2244
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2268
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2252
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2300
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2268
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2284
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2284
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2332
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2300
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2316
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2316
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2372
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2332
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2356
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2356
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2388
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2372
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2396
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2388
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2420
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2396
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2428
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2420
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2460
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2428
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2444
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2444
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2484
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2460
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2508
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2484
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2492
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2492
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2548
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2508
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2524
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2524
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2572
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2548
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2564
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2564
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2588
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2572
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2628
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2588
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2612
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2612
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2644
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2628
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2676
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2644
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2652
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2652
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2700
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2676
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2692
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2692
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2740
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2700
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2716
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2716
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2772
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2740
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2756
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2756
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2804
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2772
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2780
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2780
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2828
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2804
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2820
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2820
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2852
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2828
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2876
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2852
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2860
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2860
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2908
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2876
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2892
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2892
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2924
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2908
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2948
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2924
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2980
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2948
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2964
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2964
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2988
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2980
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3028
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2988
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3004
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3004
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3036
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3028
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3076
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3036
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3060
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3060
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3108
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3076
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3092
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3092
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3132
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3108
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3116
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3116
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3172
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3132
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3156
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3156
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3196
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3172
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3188
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3188
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3236
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3196
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3212
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3212
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3260
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3236
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3244
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3244
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3292
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3260
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3276
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3276
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3332
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3292
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3308
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3308
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3364
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3332
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3340
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3340
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3380
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3364
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3404
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3380
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3388
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3388
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3428
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3404
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3452
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3428
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3444
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3444
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3484
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3452
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3476
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3476
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3484
	add t6, sp, t6
	lw v_tmp, 0(t6)
	addw v_tmp, v_tmp, v_tmp
	sw v_tmp, 276(sp)
	lw v_tmp, 276(sp)
	sw v_tmp, 60(sp)
	lw v_tmp, 260(sp)
	sw v_tmp, 68(sp)
	lw v_tmp, 60(sp)
	sw v_tmp, 228(sp)
	lw v_tmp, 68(sp)
	sw v_tmp, 300(sp)
	lw v_tmp, 228(sp)
	sw v_tmp, 324(sp)
	lw v_tmp, 300(sp)
	sw v_tmp, 340(sp)
	lw v_tmp, 324(sp)
	sw v_tmp, 348(sp)
	lw v_tmp, 340(sp)
	sw v_tmp, 364(sp)
	lw v_tmp, 348(sp)
	sw v_tmp, 388(sp)
	lw v_tmp, 364(sp)
	sw v_tmp, 412(sp)
	lw v_tmp, 388(sp)
	sw v_tmp, 404(sp)
	lw v_tmp, 404(sp)
	sw v_tmp, 436(sp)
	lw v_tmp, 412(sp)
	sw v_tmp, 460(sp)
	lw v_tmp, 436(sp)
	sw v_tmp, 452(sp)
	lw v_tmp, 452(sp)
	sw v_tmp, 492(sp)
	lw v_tmp, 460(sp)
	sw v_tmp, 484(sp)
	lw v_tmp, 484(sp)
	sw v_tmp, 516(sp)
	lw v_tmp, 492(sp)
	sw v_tmp, 540(sp)
	lw v_tmp, 516(sp)
	sw v_tmp, 532(sp)
	lw v_tmp, 532(sp)
	sw v_tmp, 572(sp)
	lw v_tmp, 540(sp)
	sw v_tmp, 564(sp)
	lw v_tmp, 564(sp)
	sw v_tmp, 612(sp)
	lw v_tmp, 572(sp)
	sw v_tmp, 588(sp)
	lw v_tmp, 588(sp)
	sw v_tmp, 636(sp)
	lw v_tmp, 612(sp)
	sw v_tmp, 620(sp)
	lw v_tmp, 620(sp)
	sw v_tmp, 652(sp)
	lw v_tmp, 636(sp)
	sw v_tmp, 684(sp)
	lw v_tmp, 652(sp)
	sw v_tmp, 668(sp)
	lw v_tmp, 668(sp)
	sw v_tmp, 724(sp)
	lw v_tmp, 684(sp)
	sw v_tmp, 700(sp)
	lw v_tmp, 700(sp)
	sw v_tmp, 756(sp)
	lw v_tmp, 724(sp)
	sw v_tmp, 740(sp)
	lw v_tmp, 740(sp)
	sw v_tmp, 780(sp)
	lw v_tmp, 756(sp)
	sw v_tmp, 764(sp)
	lw v_tmp, 764(sp)
	sw v_tmp, 820(sp)
	lw v_tmp, 780(sp)
	sw v_tmp, 804(sp)
	lw v_tmp, 804(sp)
	sw v_tmp, 844(sp)
	lw v_tmp, 820(sp)
	sw v_tmp, 836(sp)
	lw v_tmp, 836(sp)
	sw v_tmp, 860(sp)
	lw v_tmp, 844(sp)
	sw v_tmp, 892(sp)
	lw v_tmp, 860(sp)
	sw v_tmp, 884(sp)
	lw v_tmp, 884(sp)
	sw v_tmp, 908(sp)
	lw v_tmp, 892(sp)
	sw v_tmp, 940(sp)
	lw v_tmp, 908(sp)
	sw v_tmp, 932(sp)
	lw v_tmp, 932(sp)
	sw v_tmp, 964(sp)
	lw v_tmp, 940(sp)
	sw v_tmp, 988(sp)
	lw v_tmp, 964(sp)
	sw v_tmp, 980(sp)
	lw v_tmp, 980(sp)
	sw v_tmp, 1028(sp)
	lw v_tmp, 988(sp)
	sw v_tmp, 1012(sp)
	lw v_tmp, 1012(sp)
	sw v_tmp, 1044(sp)
	lw v_tmp, 1028(sp)
	sw v_tmp, 1060(sp)
	lw v_tmp, 1044(sp)
	sw v_tmp, 1068(sp)
	lw v_tmp, 1060(sp)
	sw v_tmp, 1100(sp)
	lw v_tmp, 1068(sp)
	sw v_tmp, 1092(sp)
	lw v_tmp, 1092(sp)
	sw v_tmp, 1132(sp)
	lw v_tmp, 1100(sp)
	sw v_tmp, 1124(sp)
	lw v_tmp, 1124(sp)
	sw v_tmp, 1148(sp)
	lw v_tmp, 1132(sp)
	sw v_tmp, 1180(sp)
	lw v_tmp, 1148(sp)
	sw v_tmp, 1172(sp)
	lw v_tmp, 1172(sp)
	sw v_tmp, 1212(sp)
	lw v_tmp, 1180(sp)
	sw v_tmp, 1204(sp)
	lw v_tmp, 1204(sp)
	sw v_tmp, 1252(sp)
	lw v_tmp, 1212(sp)
	sw v_tmp, 1236(sp)
	lw v_tmp, 1236(sp)
	sw v_tmp, 1276(sp)
	lw v_tmp, 1252(sp)
	sw v_tmp, 1260(sp)
	lw v_tmp, 1260(sp)
	sw v_tmp, 1308(sp)
	lw v_tmp, 1276(sp)
	sw v_tmp, 1300(sp)
	lw v_tmp, 1300(sp)
	sw v_tmp, 1340(sp)
	lw v_tmp, 1308(sp)
	sw v_tmp, 1332(sp)
	lw v_tmp, 1332(sp)
	sw v_tmp, 1372(sp)
	lw v_tmp, 1340(sp)
	sw v_tmp, 1364(sp)
	lw v_tmp, 1364(sp)
	sw v_tmp, 1404(sp)
	lw v_tmp, 1372(sp)
	sw v_tmp, 1388(sp)
	lw v_tmp, 1388(sp)
	sw v_tmp, 1420(sp)
	lw v_tmp, 1404(sp)
	sw v_tmp, 1460(sp)
	lw v_tmp, 1420(sp)
	sw v_tmp, 1436(sp)
	lw v_tmp, 1436(sp)
	sw v_tmp, 1492(sp)
	lw v_tmp, 1460(sp)
	sw v_tmp, 1476(sp)
	lw v_tmp, 1476(sp)
	sw v_tmp, 1516(sp)
	lw v_tmp, 1492(sp)
	sw v_tmp, 1500(sp)
	lw v_tmp, 1500(sp)
	sw v_tmp, 1540(sp)
	lw v_tmp, 1516(sp)
	sw v_tmp, 1548(sp)
	lw v_tmp, 1540(sp)
	sw v_tmp, 1580(sp)
	lw v_tmp, 1548(sp)
	sw v_tmp, 1572(sp)
	lw v_tmp, 1572(sp)
	sw v_tmp, 1620(sp)
	lw v_tmp, 1580(sp)
	sw v_tmp, 1604(sp)
	lw v_tmp, 1604(sp)
	sw v_tmp, 1644(sp)
	lw v_tmp, 1620(sp)
	sw v_tmp, 1636(sp)
	lw v_tmp, 1636(sp)
	sw v_tmp, 1668(sp)
	lw v_tmp, 1644(sp)
	sw v_tmp, 1676(sp)
	lw v_tmp, 1668(sp)
	sw v_tmp, 1716(sp)
	lw v_tmp, 1676(sp)
	sw v_tmp, 1700(sp)
	lw v_tmp, 1700(sp)
	sw v_tmp, 1724(sp)
	lw v_tmp, 1716(sp)
	sw v_tmp, 1764(sp)
	lw v_tmp, 1724(sp)
	sw v_tmp, 1740(sp)
	lw v_tmp, 1740(sp)
	sw v_tmp, 1772(sp)
	lw v_tmp, 1764(sp)
	sw v_tmp, 1812(sp)
	lw v_tmp, 1772(sp)
	sw v_tmp, 1788(sp)
	lw v_tmp, 1788(sp)
	sw v_tmp, 1828(sp)
	lw v_tmp, 1812(sp)
	sw v_tmp, 1860(sp)
	lw v_tmp, 1828(sp)
	sw v_tmp, 1844(sp)
	lw v_tmp, 1844(sp)
	sw v_tmp, 1876(sp)
	lw v_tmp, 1860(sp)
	sw v_tmp, 1908(sp)
	lw v_tmp, 1876(sp)
	sw v_tmp, 1892(sp)
	lw v_tmp, 1892(sp)
	sw v_tmp, 1940(sp)
	lw v_tmp, 1908(sp)
	sw v_tmp, 1924(sp)
	lw v_tmp, 1924(sp)
	sw v_tmp, 1948(sp)
	lw v_tmp, 1940(sp)
	sw v_tmp, 1980(sp)
	lw v_tmp, 1948(sp)
	sw v_tmp, 1972(sp)
	lw v_tmp, 1972(sp)
	sw v_tmp, 1996(sp)
	lw v_tmp, 1980(sp)
	sw v_tmp, 2028(sp)
	lw v_tmp, 1996(sp)
	sw v_tmp, 2020(sp)
	lw v_tmp, 2020(sp)
	sw v_tmp, 2044(sp)
	lw v_tmp, 2028(sp)
	li t6, 2076
	add t6, sp, t6
	sw v_tmp, 0(t6)
	lw v_tmp, 2044(sp)
	li t6, 2068
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2068
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2100
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2076
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2124
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2100
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2108
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2108
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2148
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2124
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2172
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2148
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2156
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2156
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2204
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2172
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2196
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2196
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2236
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2204
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2220
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2220
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2276
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2236
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2260
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2260
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2308
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2276
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2292
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2292
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2340
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2308
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2324
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2324
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2364
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2340
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2348
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2348
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2404
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2364
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2380
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2380
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2436
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2404
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2412
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2412
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2468
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2436
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2452
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2452
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2500
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2468
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2476
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2476
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2532
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2500
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2516
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2516
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2556
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2532
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2540
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2540
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2596
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2556
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2580
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2580
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2604
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2596
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2620
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2604
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2660
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2620
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2636
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2636
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2684
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2660
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2668
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2668
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2724
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2684
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2708
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2708
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2748
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2724
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2732
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2732
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2788
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2748
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2764
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2764
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2812
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2788
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2796
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2796
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2836
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2812
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2868
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2836
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2844
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2844
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2900
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2868
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2884
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2884
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2916
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2900
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2940
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2916
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2932
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2932
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2972
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2940
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2956
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2956
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 2996
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2972
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3020
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 2996
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3012
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3012
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3052
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3020
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3044
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3044
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3068
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3052
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3100
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3068
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3084
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3084
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3124
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3100
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3148
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3124
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3140
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3140
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3180
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3148
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3164
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3164
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3220
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3180
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3204
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3204
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3252
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3220
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3228
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3228
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3268
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3252
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3300
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3268
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3284
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3284
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3324
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3300
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3316
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3316
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3356
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3324
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3348
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3348
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3396
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3356
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3372
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3372
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3420
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3396
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3412
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3412
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3436
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3420
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3460
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3436
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3468
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3460
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3492
	add t6, sp, t6
	sw v_tmp, 0(t6)
	li t6, 3468
	add t6, sp, t6
	lw v_tmp, 0(t6)
	li t6, 3492
	add t6, sp, t6
	lw v_tmp, 0(t6)
	addw v_tmp, v_tmp, v_tmp
	sw v_tmp, 236(sp)
	lw v_tmp, 236(sp)
	sw v_tmp, 76(sp)
	sw v@arg3, 84(sp)
	lw v_tmp, 76(sp)
	sw v_tmp, 244(sp)
	lw v_tmp, 84(sp)
	lw v_tmp, 244(sp)
	addw v@28, v_tmp, v_tmp
	sw v@28, 92(sp)
	lw v_tmp, 308(sp)
	sw v_tmp, 100(sp)
	lw v_tmp, 92(sp)
	lw v_tmp, 100(sp)
	addw v@30, v_tmp, v_tmp
	sw v@30, 108(sp)
	sw v@arg5, 116(sp)
	lw v_tmp, 108(sp)
	lw v_tmp, 116(sp)
	addw v@32, v_tmp, v_tmp
	sw v@32, 124(sp)
	sw v@arg6, 132(sp)
	lw v_tmp, 124(sp)
	lw v_tmp, 132(sp)
	addw v@34, v_tmp, v_tmp
	sw v@34, 140(sp)
	sw v@arg7, 148(sp)
	lw v_tmp, 140(sp)
	lw v_tmp, 148(sp)
	addw v@36, v_tmp, v_tmp
	sw v@36, 156(sp)
	li t6, 3504
	add t6, sp, t6
	lw v_tmp, 0(t6)
	sw v_tmp, 164(sp)
	lw v_tmp, 156(sp)
	lw v_tmp, 164(sp)
	addw v@38, v_tmp, v_tmp
	sw v@38, 172(sp)
	li t6, 3512
	add t6, sp, t6
	lw v_tmp, 0(t6)
	sw v_tmp, 180(sp)
	lw v_tmp, 172(sp)
	lw v_tmp, 180(sp)
	addw v@40, v_tmp, v_tmp
	sw v@40, 188(sp)
	li t6, 3520
	add t6, sp, t6
	lw v_tmp, 0(t6)
	sw v_tmp, 196(sp)
	lw v_tmp, 188(sp)
	lw v_tmp, 196(sp)
	addw v@42, v_tmp, v_tmp
	mv a0, v@42
	li t0, 3504
	add sp, sp, t0
	ret
	.size f, .-f
	.globl fWithAlloca
	.type fWithAlloca, @function
fWithAlloca:
	addi sp, sp, -1264
Label_2:
	sw a0, 0(sp)
	lw t4, 1180(sp)
	sw t4, 4(sp)
	lw t4, 1188(sp)
	sw t4, 8(sp)
	lw t4, 1132(sp)
	sw t4, 12(sp)
	sw a4, 16(sp)
	lw t4, 1148(sp)
	sw t4, 20(sp)
	lw t4, 1252(sp)
	sw t4, 24(sp)
	lw t4, 1196(sp)
	sw t4, 28(sp)
	lw t4, 1264(sp)
	sw t4, 32(sp)
	lw t4, 1272(sp)
	sw t4, 36(sp)
	lw t4, 1280(sp)
	sw t4, 40(sp)
	addi t4, sp, 44
	sd t4, 844(sp)
	addi t3, zero, 0
	lw t4, 1172(sp)
	sd t4, 852(sp)
	lw t4, 1140(sp)
	addi t4, t4, 800
	sd t4, 860(sp)
	ld t4, 852(sp)
	sw t4, 1164(sp)
	ld t4, 860(sp)
	lw t5, 1164(sp)
	mul t4, t5, t4
	add t3, t3, t4
	li t4, 150
	sd t4, 868(sp)
	lw t4, 1156(sp)
	addi t4, t4, 4
	sd t4, 876(sp)
	ld t4, 868(sp)
	ld t5, 876(sp)
	sw t5, 1124(sp)
	lw t5, 1124(sp)
	mul t4, t4, t5
	add t3, t3, t4
	ld t4, 844(sp)
	add t3, t4, t3
	sd t3, 884(sp)
	ld t3, 884(sp)
	sd t3, 892(sp)
	li t4, 9
	ld t3, 892(sp)
	sw t4, 0(t3)
	addi t3, sp, 44
	sd t3, 900(sp)
	lw t3, 1212(sp)
	addi t3, t3, 0
	lw t4, 1236(sp)
	sd t4, 908(sp)
	lw t4, 1204(sp)
	addi t4, t4, 800
	sd t4, 916(sp)
	ld t4, 908(sp)
	ld t5, 916(sp)
	sw t5, 1220(sp)
	lw t5, 1220(sp)
	mul t4, t4, t5
	add t3, t3, t4
	li t4, 150
	sd t4, 924(sp)
	lw t4, 1244(sp)
	addi t4, t4, 4
	sd t4, 932(sp)
	ld t4, 924(sp)
	ld t5, 932(sp)
	sw t5, 1228(sp)
	lw t5, 1228(sp)
	mul t4, t4, t5
	add t3, t3, t4
	ld t4, 900(sp)
	add t3, t4, t3
	sd t3, 940(sp)
	ld t3, 940(sp)
	lw t3, 0(t3)
	sw t3, 948(sp)
	sw a0, 956(sp)
	lw t3, 948(sp)
	lw t1, 956(sp)
	addw t1, t3, t1
	sw t1, 964(sp)
	lw t1, 1180(sp)
	sw t1, 972(sp)
	lw t1, 964(sp)
	lw t3, 972(sp)
	addw t1, t1, t3
	sw t1, 980(sp)
	lw t1, 1188(sp)
	sw t1, 988(sp)
	lw t1, 980(sp)
	lw t3, 988(sp)
	addw t1, t1, t3
	sw t1, 996(sp)
	lw t1, 1132(sp)
	sw t1, 1004(sp)
	lw t1, 996(sp)
	lw t3, 1004(sp)
	addw t1, t1, t3
	sw t1, 1012(sp)
	sw a4, 1020(sp)
	lw t0, 1012(sp)
	lw t1, 1020(sp)
	addw t0, t0, t1
	sw t0, 1028(sp)
	lw t0, 1148(sp)
	sw t0, 1036(sp)
	lw t0, 1028(sp)
	lw t1, 1036(sp)
	addw t0, t0, t1
	sw t0, 1044(sp)
	lw t0, 1252(sp)
	sw t0, 1052(sp)
	lw t0, 1044(sp)
	lw t1, 1052(sp)
	addw t0, t0, t1
	sw t0, 1060(sp)
	lw t0, 1196(sp)
	sw t0, 1068(sp)
	lw t0, 1060(sp)
	lw t1, 1068(sp)
	addw t0, t0, t1
	sw t0, 1076(sp)
	lw t0, 1264(sp)
	sw t0, 1084(sp)
	lw t1, 1076(sp)
	lw t0, 1084(sp)
	addw t0, t1, t0
	sw t0, 1092(sp)
	lw t0, 1272(sp)
	sw t0, 1100(sp)
	lw t1, 1092(sp)
	lw t0, 1100(sp)
	addw t0, t1, t0
	sw t0, 1108(sp)
	lw t0, 1280(sp)
	sw t0, 1116(sp)
	lw t0, 1108(sp)
	lw t1, 1116(sp)
	addw t0, t0, t1
	mv a0, t0
	addi sp, sp, 1264
	ret
	.size fWithAlloca, .-fWithAlloca
	.globl main
	.type main, @function
main:
	addi sp, sp, -224
	sd ra, 208(sp)
Label_3:
	li t2, 8
	sw t2, 0(sp)
	li t2, 7
	sw t2, 8(sp)
	li t2, 6
	sw t2, 16(sp)
	li t2, 5
	sw t2, 24(sp)
	li t2, 4
	sw t2, 32(sp)
	li t2, 3
	sw t2, 40(sp)
	li t2, 2
	sw t2, 48(sp)
	li t2, 1
	sw t2, 56(sp)
	li t2, 11
	sw t2, 64(sp)
	li t2, 10
	sw t2, 72(sp)
	li t2, 9
	sw t2, 80(sp)
	addi sp, sp, -128
	lw t2, 208(sp)
	sw t2, 0(sp)
	lw t2, 200(sp)
	sw t2, 8(sp)
	lw t2, 192(sp)
	sw t2, 16(sp)
	sd t0, 24(sp)
	sd t1, 32(sp)
	sd t2, 40(sp)
	sd t3, 48(sp)
	sd t4, 56(sp)
	sd t5, 64(sp)
	sd t6, 72(sp)
	fsw ft5, 88(sp)
	fsw ft6, 92(sp)
	fsw ft7, 96(sp)
	fsw ft8, 100(sp)
	fsw ft9, 104(sp)
	fsw ft10, 108(sp)
	fsw ft11, 112(sp)
	lw t2, 184(sp)
	mv a0, t2
	lw t2, 176(sp)
	mv a1, t2
	lw t2, 168(sp)
	mv a2, t2
	lw t2, 160(sp)
	mv a3, t2
	lw t2, 152(sp)
	mv a4, t2
	lw t2, 144(sp)
	mv a5, t2
	lw t2, 136(sp)
	mv a6, t2
	lw t2, 128(sp)
	mv a7, t2
	call f
	ld t0, 24(sp)
	ld t1, 32(sp)
	ld t2, 40(sp)
	ld t3, 48(sp)
	ld t4, 56(sp)
	ld t5, 64(sp)
	ld t6, 72(sp)
	flw ft5, 88(sp)
	flw ft6, 92(sp)
	flw ft7, 96(sp)
	flw ft8, 100(sp)
	flw ft9, 104(sp)
	flw ft10, 108(sp)
	flw ft11, 112(sp)
	addi sp, sp, 128
	mv t2, a0
	sw t2, 88(sp)
	addi sp, sp, -192
	sd t0, 0(sp)
	sd t1, 8(sp)
	sd t2, 16(sp)
	sd t3, 24(sp)
	sd t4, 32(sp)
	sd t5, 40(sp)
	sd t6, 48(sp)
	sd a0, 64(sp)
	sd a1, 72(sp)
	sd a2, 80(sp)
	sd a3, 88(sp)
	sd a4, 96(sp)
	sd a5, 104(sp)
	sd a6, 112(sp)
	sd a7, 120(sp)
	fsw ft5, 128(sp)
	fsw ft6, 132(sp)
	fsw ft7, 136(sp)
	fsw ft8, 140(sp)
	fsw ft9, 144(sp)
	fsw ft10, 148(sp)
	fsw ft11, 152(sp)
	fsw fa0, 160(sp)
	fsw fa1, 164(sp)
	fsw fa2, 168(sp)
	fsw fa3, 172(sp)
	fsw fa4, 176(sp)
	fsw fa5, 180(sp)
	fsw fa6, 184(sp)
	fsw fa7, 188(sp)
	lw t2, 280(sp)
	mv a0, t2
	call putint
	ld t0, 0(sp)
	ld t1, 8(sp)
	ld t2, 16(sp)
	ld t3, 24(sp)
	ld t4, 32(sp)
	ld t5, 40(sp)
	ld t6, 48(sp)
	ld a0, 64(sp)
	ld a1, 72(sp)
	ld a2, 80(sp)
	ld a3, 88(sp)
	ld a4, 96(sp)
	ld a5, 104(sp)
	ld a6, 112(sp)
	ld a7, 120(sp)
	flw ft5, 128(sp)
	flw ft6, 132(sp)
	flw ft7, 136(sp)
	flw ft8, 140(sp)
	flw ft9, 144(sp)
	flw ft10, 148(sp)
	flw ft11, 152(sp)
	flw fa0, 160(sp)
	flw fa1, 164(sp)
	flw fa2, 168(sp)
	flw fa3, 172(sp)
	flw fa4, 176(sp)
	flw fa5, 180(sp)
	flw fa6, 184(sp)
	flw fa7, 188(sp)
	addi sp, sp, 192
	li t2, 10
	sw t2, 96(sp)
	addi sp, sp, -192
	sd t0, 0(sp)
	sd t1, 8(sp)
	sd t2, 16(sp)
	sd t3, 24(sp)
	sd t4, 32(sp)
	sd t5, 40(sp)
	sd t6, 48(sp)
	sd a0, 64(sp)
	sd a1, 72(sp)
	sd a2, 80(sp)
	sd a3, 88(sp)
	sd a4, 96(sp)
	sd a5, 104(sp)
	sd a6, 112(sp)
	sd a7, 120(sp)
	fsw ft5, 128(sp)
	fsw ft6, 132(sp)
	fsw ft7, 136(sp)
	fsw ft8, 140(sp)
	fsw ft9, 144(sp)
	fsw ft10, 148(sp)
	fsw ft11, 152(sp)
	fsw fa0, 160(sp)
	fsw fa1, 164(sp)
	fsw fa2, 168(sp)
	fsw fa3, 172(sp)
	fsw fa4, 176(sp)
	fsw fa5, 180(sp)
	fsw fa6, 184(sp)
	fsw fa7, 188(sp)
	lw t2, 288(sp)
	mv a0, t2
	call putch
	ld t0, 0(sp)
	ld t1, 8(sp)
	ld t2, 16(sp)
	ld t3, 24(sp)
	ld t4, 32(sp)
	ld t5, 40(sp)
	ld t6, 48(sp)
	ld a0, 64(sp)
	ld a1, 72(sp)
	ld a2, 80(sp)
	ld a3, 88(sp)
	ld a4, 96(sp)
	ld a5, 104(sp)
	ld a6, 112(sp)
	ld a7, 120(sp)
	flw ft5, 128(sp)
	flw ft6, 132(sp)
	flw ft7, 136(sp)
	flw ft8, 140(sp)
	flw ft9, 144(sp)
	flw ft10, 148(sp)
	flw ft11, 152(sp)
	flw fa0, 160(sp)
	flw fa1, 164(sp)
	flw fa2, 168(sp)
	flw fa3, 172(sp)
	flw fa4, 176(sp)
	flw fa5, 180(sp)
	flw fa6, 184(sp)
	flw fa7, 188(sp)
	addi sp, sp, 192
	li t2, 8
	sw t2, 104(sp)
	li t2, 7
	sw t2, 112(sp)
	li t2, 6
	sw t2, 120(sp)
	li t2, 5
	sw t2, 128(sp)
	li t2, 4
	sw t2, 136(sp)
	li t2, 3
	sw t2, 144(sp)
	li t2, 2
	sw t2, 152(sp)
	li t2, 1
	sw t2, 160(sp)
	li t2, 11
	sw t2, 168(sp)
	li t2, 10
	sw t2, 176(sp)
	li t2, 9
	sw t2, 184(sp)
	addi sp, sp, -128
	lw t2, 312(sp)
	sw t2, 0(sp)
	lw t2, 304(sp)
	sw t2, 8(sp)
	lw t2, 296(sp)
	sw t2, 16(sp)
	sd t0, 24(sp)
	sd t1, 32(sp)
	sd t2, 40(sp)
	sd t3, 48(sp)
	sd t4, 56(sp)
	sd t5, 64(sp)
	sd t6, 72(sp)
	fsw ft5, 88(sp)
	fsw ft6, 92(sp)
	fsw ft7, 96(sp)
	fsw ft8, 100(sp)
	fsw ft9, 104(sp)
	fsw ft10, 108(sp)
	fsw ft11, 112(sp)
	lw t2, 288(sp)
	mv a0, t2
	lw t2, 280(sp)
	mv a1, t2
	lw t2, 272(sp)
	mv a2, t2
	lw t2, 264(sp)
	mv a3, t2
	lw t2, 256(sp)
	mv a4, t2
	lw t2, 248(sp)
	mv a5, t2
	lw t2, 240(sp)
	mv a6, t2
	lw t2, 232(sp)
	mv a7, t2
	call fWithAlloca
	ld t0, 24(sp)
	ld t1, 32(sp)
	ld t2, 40(sp)
	ld t3, 48(sp)
	ld t4, 56(sp)
	ld t5, 64(sp)
	ld t6, 72(sp)
	flw ft5, 88(sp)
	flw ft6, 92(sp)
	flw ft7, 96(sp)
	flw ft8, 100(sp)
	flw ft9, 104(sp)
	flw ft10, 108(sp)
	flw ft11, 112(sp)
	addi sp, sp, 128
	mv t2, a0
	sw t2, 192(sp)
	addi sp, sp, -192
	sd t0, 0(sp)
	sd t1, 8(sp)
	sd t2, 16(sp)
	sd t3, 24(sp)
	sd t4, 32(sp)
	sd t5, 40(sp)
	sd t6, 48(sp)
	sd a0, 64(sp)
	sd a1, 72(sp)
	sd a2, 80(sp)
	sd a3, 88(sp)
	sd a4, 96(sp)
	sd a5, 104(sp)
	sd a6, 112(sp)
	sd a7, 120(sp)
	fsw ft5, 128(sp)
	fsw ft6, 132(sp)
	fsw ft7, 136(sp)
	fsw ft8, 140(sp)
	fsw ft9, 144(sp)
	fsw ft10, 148(sp)
	fsw ft11, 152(sp)
	fsw fa0, 160(sp)
	fsw fa1, 164(sp)
	fsw fa2, 168(sp)
	fsw fa3, 172(sp)
	fsw fa4, 176(sp)
	fsw fa5, 180(sp)
	fsw fa6, 184(sp)
	fsw fa7, 188(sp)
	lw t2, 384(sp)
	mv a0, t2
	call putint
	ld t0, 0(sp)
	ld t1, 8(sp)
	ld t2, 16(sp)
	ld t3, 24(sp)
	ld t4, 32(sp)
	ld t5, 40(sp)
	ld t6, 48(sp)
	ld a0, 64(sp)
	ld a1, 72(sp)
	ld a2, 80(sp)
	ld a3, 88(sp)
	ld a4, 96(sp)
	ld a5, 104(sp)
	ld a6, 112(sp)
	ld a7, 120(sp)
	flw ft5, 128(sp)
	flw ft6, 132(sp)
	flw ft7, 136(sp)
	flw ft8, 140(sp)
	flw ft9, 144(sp)
	flw ft10, 148(sp)
	flw ft11, 152(sp)
	flw fa0, 160(sp)
	flw fa1, 164(sp)
	flw fa2, 168(sp)
	flw fa3, 172(sp)
	flw fa4, 176(sp)
	flw fa5, 180(sp)
	flw fa6, 184(sp)
	flw fa7, 188(sp)
	addi sp, sp, 192
	li t2, 10
	sw t2, 200(sp)
	addi sp, sp, -192
	sd t0, 0(sp)
	sd t1, 8(sp)
	sd t2, 16(sp)
	sd t3, 24(sp)
	sd t4, 32(sp)
	sd t5, 40(sp)
	sd t6, 48(sp)
	sd a0, 64(sp)
	sd a1, 72(sp)
	sd a2, 80(sp)
	sd a3, 88(sp)
	sd a4, 96(sp)
	sd a5, 104(sp)
	sd a6, 112(sp)
	sd a7, 120(sp)
	fsw ft5, 128(sp)
	fsw ft6, 132(sp)
	fsw ft7, 136(sp)
	fsw ft8, 140(sp)
	fsw ft9, 144(sp)
	fsw ft10, 148(sp)
	fsw ft11, 152(sp)
	fsw fa0, 160(sp)
	fsw fa1, 164(sp)
	fsw fa2, 168(sp)
	fsw fa3, 172(sp)
	fsw fa4, 176(sp)
	fsw fa5, 180(sp)
	fsw fa6, 184(sp)
	fsw fa7, 188(sp)
	lw t2, 392(sp)
	mv a0, t2
	call putch
	ld t0, 0(sp)
	ld t1, 8(sp)
	ld t2, 16(sp)
	ld t3, 24(sp)
	ld t4, 32(sp)
	ld t5, 40(sp)
	ld t6, 48(sp)
	ld a0, 64(sp)
	ld a1, 72(sp)
	ld a2, 80(sp)
	ld a3, 88(sp)
	ld a4, 96(sp)
	ld a5, 104(sp)
	ld a6, 112(sp)
	ld a7, 120(sp)
	flw ft5, 128(sp)
	flw ft6, 132(sp)
	flw ft7, 136(sp)
	flw ft8, 140(sp)
	flw ft9, 144(sp)
	flw ft10, 148(sp)
	flw ft11, 152(sp)
	flw fa0, 160(sp)
	flw fa1, 164(sp)
	flw fa2, 168(sp)
	flw fa3, 172(sp)
	flw fa4, 176(sp)
	flw fa5, 180(sp)
	flw fa6, 184(sp)
	flw fa7, 188(sp)
	addi sp, sp, 192
	mv a0, zero
	ld ra, 208(sp)
	addi sp, sp, 224
	ret
	.size main, .-main

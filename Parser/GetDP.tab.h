#ifndef BISON_GETDP_TAB_H
# define BISON_GETDP_TAB_H

#ifndef YYSTYPE
typedef union {
  char    *c ;
  int     i ;
  double  d ;
  List_T  *l ;
  struct TwoInt t ;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	tINT	257
# define	tFLOAT	258
# define	tSTRING	259
# define	tBIGSTR	260
# define	tEND	261
# define	tDOTS	262
# define	tStrCat	263
# define	tInclude	264
# define	tConstant	265
# define	tList	266
# define	tListAlt	267
# define	tLinSpace	268
# define	tLogSpace	269
# define	tDefineConstant	270
# define	tPi	271
# define	t0D	272
# define	t1D	273
# define	t2D	274
# define	t3D	275
# define	tExp	276
# define	tLog	277
# define	tLog10	278
# define	tSqrt	279
# define	tSin	280
# define	tAsin	281
# define	tCos	282
# define	tAcos	283
# define	tTan	284
# define	tAtan	285
# define	tAtan2	286
# define	tSinh	287
# define	tCosh	288
# define	tTanh	289
# define	tFabs	290
# define	tFloor	291
# define	tCeil	292
# define	tFmod	293
# define	tModulo	294
# define	tHypot	295
# define	tSolidAngle	296
# define	tTrace	297
# define	tOrder	298
# define	tCrossProduct	299
# define	tMHTransform	300
# define	tMHJacNL	301
# define	tGroup	302
# define	tDefineGroup	303
# define	tAll	304
# define	tInSupport	305
# define	tMovingBand2D	306
# define	tDefineFunction	307
# define	tConstraint	308
# define	tRegion	309
# define	tSubRegion	310
# define	tRegionRef	311
# define	tSubRegionRef	312
# define	tFilter	313
# define	tCoefficient	314
# define	tValue	315
# define	tTimeFunction	316
# define	tBranch	317
# define	tNode	318
# define	tLoop	319
# define	tNameOfResolution	320
# define	tJacobian	321
# define	tCase	322
# define	tIntegration	323
# define	tType	324
# define	tSubType	325
# define	tCriterion	326
# define	tGeoElement	327
# define	tNumberOfPoints	328
# define	tMaxNumberOfPoints	329
# define	tNumberOfDivisions	330
# define	tMaxNumberOfDivisions	331
# define	tStoppingCriterion	332
# define	tFunctionSpace	333
# define	tName	334
# define	tBasisFunction	335
# define	tNameOfCoef	336
# define	tFunction	337
# define	tdFunction	338
# define	tSubFunction	339
# define	tSupport	340
# define	tEntity	341
# define	tSubSpace	342
# define	tNameOfBasisFunction	343
# define	tGlobalQuantity	344
# define	tEntityType	345
# define	tEntitySubType	346
# define	tNameOfConstraint	347
# define	tFormulation	348
# define	tQuantity	349
# define	tNameOfSpace	350
# define	tIndexOfSystem	351
# define	tSymmetry	352
# define	tEquation	353
# define	tGalerkin	354
# define	tdeRham	355
# define	tGlobalTerm	356
# define	tGlobalEquation	357
# define	tDt	358
# define	tDtDof	359
# define	tDtDt	360
# define	tDtDtDof	361
# define	tJacNL	362
# define	tNeverDt	363
# define	tIn	364
# define	tResolution	365
# define	tDefineSystem	366
# define	tNameOfFormulation	367
# define	tNameOfMesh	368
# define	tFrequency	369
# define	tSolver	370
# define	tOriginSystem	371
# define	tDestinationSystem	372
# define	tOperation	373
# define	tOperationEnd	374
# define	tSetTime	375
# define	tDTime	376
# define	tSetFrequency	377
# define	tFourierTransform	378
# define	tFourierTransformJ	379
# define	tIf	380
# define	tElse	381
# define	tLanczos	382
# define	tPerturbation	383
# define	tUpdate	384
# define	tUpdateConstraint	385
# define	tBreak	386
# define	tTimeLoopTheta	387
# define	tTime0	388
# define	tTimeMax	389
# define	tTheta	390
# define	tTimeLoopNewmark	391
# define	tBeta	392
# define	tGamma	393
# define	tIterativeLoop	394
# define	tNbrMaxIteration	395
# define	tRelaxationFactor	396
# define	tIterativeTimeReduction	397
# define	tDivisionCoefficient	398
# define	tChangeOfState	399
# define	tChangeOfCoordinates	400
# define	tSystemCommand	401
# define	tSolveJac_AdaptRelax	402
# define	tSaveSolutionExtendedMH	403
# define	tInit_MovingBand2D	404
# define	tMesh_MovingBand2D	405
# define	tGenerate_MH_Moving	406
# define	tGenerateGroup	407
# define	tGenerateJacGroup	408
# define	tSaveMesh	409
# define	tPostProcessing	410
# define	tNameOfSystem	411
# define	tPostOperation	412
# define	tNameOfPostProcessing	413
# define	tUsingPost	414
# define	tAppend	415
# define	tPlot	416
# define	tPrint	417
# define	tPrintGroup	418
# define	tEcho	419
# define	tWrite	420
# define	tAdapt	421
# define	tOnGlobal	422
# define	tOnRegion	423
# define	tOnElementsOf	424
# define	tOnGrid	425
# define	tOnSection	426
# define	tOnPoint	427
# define	tOnLine	428
# define	tOnPlane	429
# define	tOnBox	430
# define	tWithArgument	431
# define	tFile	432
# define	tDepth	433
# define	tDimension	434
# define	tTimeStep	435
# define	tHarmonicToTime	436
# define	tFormat	437
# define	tHeader	438
# define	tFooter	439
# define	tSkin	440
# define	tSmoothing	441
# define	tTarget	442
# define	tSort	443
# define	tIso	444
# define	tNoNewLine	445
# define	tDecomposeInSimplex	446
# define	tChangeOfValues	447
# define	tFlag	448
# define	tHelp	449
# define	tCpu	450
# define	tCheck	451
# define	tDEF	452
# define	tOR	453
# define	tAND	454
# define	tEQUAL	455
# define	tNOTEQUAL	456
# define	tAPPROXEQUAL	457
# define	tLESSOREQUAL	458
# define	tGREATEROREQUAL	459
# define	tLESSLESS	460
# define	tGREATERGREATER	461
# define	tCROSSPRODUCT	462
# define	UNARYPREC	463
# define	tSHOW	464


extern YYSTYPE yylval;

#endif /* not BISON_GETDP_TAB_H */

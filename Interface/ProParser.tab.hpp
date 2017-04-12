/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tINT = 258,
     tFLOAT = 259,
     tSTRING = 260,
     tBIGSTR = 261,
     tEND = 262,
     tDOTS = 263,
     tSCOPE = 264,
     tStr = 265,
     tStrList = 266,
     tStrCat = 267,
     tSprintf = 268,
     tPrintf = 269,
     tMPI_Printf = 270,
     tRead = 271,
     tPrintConstants = 272,
     tStrCmp = 273,
     tStrFind = 274,
     tStrLen = 275,
     tStrChoice = 276,
     tStrSub = 277,
     tUpperCase = 278,
     tLowerCase = 279,
     tLowerCaseIn = 280,
     tNbrRegions = 281,
     tGetRegion = 282,
     tGetRegions = 283,
     tStringToName = 284,
     tNameToString = 285,
     tFor = 286,
     tEndFor = 287,
     tIf = 288,
     tElseIf = 289,
     tElse = 290,
     tEndIf = 291,
     tMacro = 292,
     tReturn = 293,
     tCall = 294,
     tCallTest = 295,
     tTest = 296,
     tWhile = 297,
     tParse = 298,
     tFlag = 299,
     tExists = 300,
     tFileExists = 301,
     tGetForced = 302,
     tGetForcedStr = 303,
     tInclude = 304,
     tLevelInclude = 305,
     tConstant = 306,
     tList = 307,
     tListAlt = 308,
     tLinSpace = 309,
     tLogSpace = 310,
     tListFromFile = 311,
     tChangeCurrentPosition = 312,
     tDefineConstant = 313,
     tUndefineConstant = 314,
     tDefineNumber = 315,
     tDefineString = 316,
     tDefineStruct = 317,
     tNameStruct = 318,
     tDimNameSpace = 319,
     tGetNumber = 320,
     tGetString = 321,
     tSetNumber = 322,
     tSetString = 323,
     tPi = 324,
     tMPI_Rank = 325,
     tMPI_Size = 326,
     t0D = 327,
     t1D = 328,
     t2D = 329,
     t3D = 330,
     tLevelTest = 331,
     tTotalMemory = 332,
     tNumInclude = 333,
     tCurrentDirectory = 334,
     tAbsolutePath = 335,
     tDirName = 336,
     tBaseFileName = 337,
     tCurrentFileName = 338,
     tGETDP_MAJOR_VERSION = 339,
     tGETDP_MINOR_VERSION = 340,
     tGETDP_PATCH_VERSION = 341,
     tExp = 342,
     tLog = 343,
     tLog10 = 344,
     tSqrt = 345,
     tSin = 346,
     tAsin = 347,
     tCos = 348,
     tAcos = 349,
     tTan = 350,
     tAtan = 351,
     tAtan2 = 352,
     tSinh = 353,
     tCosh = 354,
     tTanh = 355,
     tFabs = 356,
     tFloor = 357,
     tCeil = 358,
     tRound = 359,
     tSign = 360,
     tFmod = 361,
     tModulo = 362,
     tHypot = 363,
     tRand = 364,
     tSolidAngle = 365,
     tTrace = 366,
     tOrder = 367,
     tCrossProduct = 368,
     tDofValue = 369,
     tMHTransform = 370,
     tMHJacNL = 371,
     tAppend = 372,
     tGroup = 373,
     tDefineGroup = 374,
     tAll = 375,
     tInSupport = 376,
     tMovingBand2D = 377,
     tDefineFunction = 378,
     tUndefineFunction = 379,
     tConstraint = 380,
     tRegion = 381,
     tSubRegion = 382,
     tSubRegion2 = 383,
     tRegionRef = 384,
     tSubRegionRef = 385,
     tFilter = 386,
     tToleranceFactor = 387,
     tCoefficient = 388,
     tValue = 389,
     tTimeFunction = 390,
     tBranch = 391,
     tNameOfResolution = 392,
     tJacobian = 393,
     tCase = 394,
     tMetricTensor = 395,
     tIntegration = 396,
     tType = 397,
     tSubType = 398,
     tCriterion = 399,
     tGeoElement = 400,
     tNumberOfPoints = 401,
     tMaxNumberOfPoints = 402,
     tNumberOfDivisions = 403,
     tMaxNumberOfDivisions = 404,
     tStoppingCriterion = 405,
     tFunctionSpace = 406,
     tName = 407,
     tBasisFunction = 408,
     tNameOfCoef = 409,
     tFunction = 410,
     tdFunction = 411,
     tSubFunction = 412,
     tSubdFunction = 413,
     tSupport = 414,
     tEntity = 415,
     tSubSpace = 416,
     tNameOfBasisFunction = 417,
     tGlobalQuantity = 418,
     tEntityType = 419,
     tAuto = 420,
     tEntitySubType = 421,
     tNameOfConstraint = 422,
     tFormulation = 423,
     tQuantity = 424,
     tNameOfSpace = 425,
     tIndexOfSystem = 426,
     tSymmetry = 427,
     tGalerkin = 428,
     tdeRham = 429,
     tGlobalTerm = 430,
     tGlobalEquation = 431,
     tDt = 432,
     tDtDof = 433,
     tDtDt = 434,
     tDtDtDof = 435,
     tDtDtDtDof = 436,
     tDtDtDtDtDof = 437,
     tDtDtDtDtDtDof = 438,
     tNLEig1Dof = 439,
     tNLEig2Dof = 440,
     tNLEig3Dof = 441,
     tNLEig4Dof = 442,
     tNLEig5Dof = 443,
     tNLEig6Dof = 444,
     tJacNL = 445,
     tDtDofJacNL = 446,
     tNeverDt = 447,
     tDtNL = 448,
     tAtAnteriorTimeStep = 449,
     tMaxOverTime = 450,
     tFourierSteinmetz = 451,
     tIn = 452,
     tFull_Matrix = 453,
     tResolution = 454,
     tHidden = 455,
     tDefineSystem = 456,
     tNameOfFormulation = 457,
     tNameOfMesh = 458,
     tFrequency = 459,
     tSolver = 460,
     tOriginSystem = 461,
     tDestinationSystem = 462,
     tOperation = 463,
     tOperationEnd = 464,
     tSetTime = 465,
     tSetTimeStep = 466,
     tSetDTime = 467,
     tDTime = 468,
     tSetFrequency = 469,
     tFourierTransform = 470,
     tFourierTransformJ = 471,
     tCopySolution = 472,
     tCopyRHS = 473,
     tCopyResidual = 474,
     tCopyIncrement = 475,
     tCopyDofs = 476,
     tGetNormSolution = 477,
     tGetNormResidual = 478,
     tGetNormRHS = 479,
     tGetNormIncrement = 480,
     tLanczos = 481,
     tEigenSolve = 482,
     tEigenSolveJac = 483,
     tPerturbation = 484,
     tUpdate = 485,
     tUpdateConstraint = 486,
     tBreak = 487,
     tGetResidual = 488,
     tCreateSolution = 489,
     tEvaluate = 490,
     tSelectCorrection = 491,
     tAddCorrection = 492,
     tMultiplySolution = 493,
     tAddOppositeFullSolution = 494,
     tSolveAgainWithOther = 495,
     tSetGlobalSolverOptions = 496,
     tTimeLoopTheta = 497,
     tTimeLoopNewmark = 498,
     tTimeLoopRungeKutta = 499,
     tTimeLoopAdaptive = 500,
     tTime0 = 501,
     tTimeMax = 502,
     tTheta = 503,
     tBeta = 504,
     tGamma = 505,
     tIterativeLoop = 506,
     tIterativeLoopN = 507,
     tIterativeLinearSolver = 508,
     tNbrMaxIteration = 509,
     tRelaxationFactor = 510,
     tIterativeTimeReduction = 511,
     tSetCommSelf = 512,
     tSetCommWorld = 513,
     tBarrier = 514,
     tBroadcastFields = 515,
     tBroadcastVariables = 516,
     tSleep = 517,
     tDivisionCoefficient = 518,
     tChangeOfState = 519,
     tChangeOfCoordinates = 520,
     tChangeOfCoordinates2 = 521,
     tSystemCommand = 522,
     tError = 523,
     tGmshRead = 524,
     tGmshMerge = 525,
     tGmshOpen = 526,
     tGmshWrite = 527,
     tGmshClearAll = 528,
     tDelete = 529,
     tDeleteFile = 530,
     tRenameFile = 531,
     tCreateDir = 532,
     tGenerateOnly = 533,
     tGenerateOnlyJac = 534,
     tSolveJac_AdaptRelax = 535,
     tSaveSolutionExtendedMH = 536,
     tSaveSolutionMHtoTime = 537,
     tSaveSolutionWithEntityNum = 538,
     tInitMovingBand2D = 539,
     tMeshMovingBand2D = 540,
     tGenerateMHMoving = 541,
     tGenerateMHMovingSeparate = 542,
     tAddMHMoving = 543,
     tGenerateGroup = 544,
     tGenerateJacGroup = 545,
     tGenerateRHSGroup = 546,
     tGenerateGroupCumulative = 547,
     tGenerateJacGroupCumulative = 548,
     tGenerateRHSGroupCumulative = 549,
     tSaveMesh = 550,
     tDeformMesh = 551,
     tFrequencySpectrum = 552,
     tPostProcessing = 553,
     tNameOfSystem = 554,
     tPostOperation = 555,
     tNameOfPostProcessing = 556,
     tUsingPost = 557,
     tResampleTime = 558,
     tPlot = 559,
     tPrint = 560,
     tPrintGroup = 561,
     tEcho = 562,
     tSendMergeFileRequest = 563,
     tWrite = 564,
     tAdapt = 565,
     tOnGlobal = 566,
     tOnRegion = 567,
     tOnElementsOf = 568,
     tOnGrid = 569,
     tOnSection = 570,
     tOnPoint = 571,
     tOnLine = 572,
     tOnPlane = 573,
     tOnBox = 574,
     tWithArgument = 575,
     tFile = 576,
     tDepth = 577,
     tDimension = 578,
     tComma = 579,
     tTimeStep = 580,
     tHarmonicToTime = 581,
     tCosineTransform = 582,
     tTimeToHarmonic = 583,
     tValueIndex = 584,
     tValueName = 585,
     tFormat = 586,
     tHeader = 587,
     tFooter = 588,
     tSkin = 589,
     tSmoothing = 590,
     tTarget = 591,
     tSort = 592,
     tIso = 593,
     tNoNewLine = 594,
     tNoTitle = 595,
     tDecomposeInSimplex = 596,
     tChangeOfValues = 597,
     tTimeLegend = 598,
     tFrequencyLegend = 599,
     tEigenvalueLegend = 600,
     tEvaluationPoints = 601,
     tStoreInRegister = 602,
     tStoreInVariable = 603,
     tStoreInField = 604,
     tStoreInMeshBasedField = 605,
     tStoreMaxInRegister = 606,
     tStoreMaxXinRegister = 607,
     tStoreMaxYinRegister = 608,
     tStoreMaxZinRegister = 609,
     tStoreMinInRegister = 610,
     tStoreMinXinRegister = 611,
     tStoreMinYinRegister = 612,
     tStoreMinZinRegister = 613,
     tLastTimeStepOnly = 614,
     tAppendTimeStepToFileName = 615,
     tTimeValue = 616,
     tTimeImagValue = 617,
     tTimeInterval = 618,
     tAppendExpressionToFileName = 619,
     tAppendExpressionFormat = 620,
     tOverrideTimeStepValue = 621,
     tNoMesh = 622,
     tSendToServer = 623,
     tDate = 624,
     tOnelabAction = 625,
     tCodeName = 626,
     tFixRelativePath = 627,
     tAppendToExistingFile = 628,
     tAppendStringToFileName = 629,
     tDEF = 630,
     tOR = 631,
     tAND = 632,
     tAPPROXEQUAL = 633,
     tNOTEQUAL = 634,
     tEQUAL = 635,
     tGREATERGREATER = 636,
     tLESSLESS = 637,
     tGREATEROREQUAL = 638,
     tLESSOREQUAL = 639,
     tCROSSPRODUCT = 640,
     UNARYPREC = 641,
     tSHOW = 642
   };
#endif
/* Tokens.  */
#define tINT 258
#define tFLOAT 259
#define tSTRING 260
#define tBIGSTR 261
#define tEND 262
#define tDOTS 263
#define tSCOPE 264
#define tStr 265
#define tStrList 266
#define tStrCat 267
#define tSprintf 268
#define tPrintf 269
#define tMPI_Printf 270
#define tRead 271
#define tPrintConstants 272
#define tStrCmp 273
#define tStrFind 274
#define tStrLen 275
#define tStrChoice 276
#define tStrSub 277
#define tUpperCase 278
#define tLowerCase 279
#define tLowerCaseIn 280
#define tNbrRegions 281
#define tGetRegion 282
#define tGetRegions 283
#define tStringToName 284
#define tNameToString 285
#define tFor 286
#define tEndFor 287
#define tIf 288
#define tElseIf 289
#define tElse 290
#define tEndIf 291
#define tMacro 292
#define tReturn 293
#define tCall 294
#define tCallTest 295
#define tTest 296
#define tWhile 297
#define tParse 298
#define tFlag 299
#define tExists 300
#define tFileExists 301
#define tGetForced 302
#define tGetForcedStr 303
#define tInclude 304
#define tLevelInclude 305
#define tConstant 306
#define tList 307
#define tListAlt 308
#define tLinSpace 309
#define tLogSpace 310
#define tListFromFile 311
#define tChangeCurrentPosition 312
#define tDefineConstant 313
#define tUndefineConstant 314
#define tDefineNumber 315
#define tDefineString 316
#define tDefineStruct 317
#define tNameStruct 318
#define tDimNameSpace 319
#define tGetNumber 320
#define tGetString 321
#define tSetNumber 322
#define tSetString 323
#define tPi 324
#define tMPI_Rank 325
#define tMPI_Size 326
#define t0D 327
#define t1D 328
#define t2D 329
#define t3D 330
#define tLevelTest 331
#define tTotalMemory 332
#define tNumInclude 333
#define tCurrentDirectory 334
#define tAbsolutePath 335
#define tDirName 336
#define tBaseFileName 337
#define tCurrentFileName 338
#define tGETDP_MAJOR_VERSION 339
#define tGETDP_MINOR_VERSION 340
#define tGETDP_PATCH_VERSION 341
#define tExp 342
#define tLog 343
#define tLog10 344
#define tSqrt 345
#define tSin 346
#define tAsin 347
#define tCos 348
#define tAcos 349
#define tTan 350
#define tAtan 351
#define tAtan2 352
#define tSinh 353
#define tCosh 354
#define tTanh 355
#define tFabs 356
#define tFloor 357
#define tCeil 358
#define tRound 359
#define tSign 360
#define tFmod 361
#define tModulo 362
#define tHypot 363
#define tRand 364
#define tSolidAngle 365
#define tTrace 366
#define tOrder 367
#define tCrossProduct 368
#define tDofValue 369
#define tMHTransform 370
#define tMHJacNL 371
#define tAppend 372
#define tGroup 373
#define tDefineGroup 374
#define tAll 375
#define tInSupport 376
#define tMovingBand2D 377
#define tDefineFunction 378
#define tUndefineFunction 379
#define tConstraint 380
#define tRegion 381
#define tSubRegion 382
#define tSubRegion2 383
#define tRegionRef 384
#define tSubRegionRef 385
#define tFilter 386
#define tToleranceFactor 387
#define tCoefficient 388
#define tValue 389
#define tTimeFunction 390
#define tBranch 391
#define tNameOfResolution 392
#define tJacobian 393
#define tCase 394
#define tMetricTensor 395
#define tIntegration 396
#define tType 397
#define tSubType 398
#define tCriterion 399
#define tGeoElement 400
#define tNumberOfPoints 401
#define tMaxNumberOfPoints 402
#define tNumberOfDivisions 403
#define tMaxNumberOfDivisions 404
#define tStoppingCriterion 405
#define tFunctionSpace 406
#define tName 407
#define tBasisFunction 408
#define tNameOfCoef 409
#define tFunction 410
#define tdFunction 411
#define tSubFunction 412
#define tSubdFunction 413
#define tSupport 414
#define tEntity 415
#define tSubSpace 416
#define tNameOfBasisFunction 417
#define tGlobalQuantity 418
#define tEntityType 419
#define tAuto 420
#define tEntitySubType 421
#define tNameOfConstraint 422
#define tFormulation 423
#define tQuantity 424
#define tNameOfSpace 425
#define tIndexOfSystem 426
#define tSymmetry 427
#define tGalerkin 428
#define tdeRham 429
#define tGlobalTerm 430
#define tGlobalEquation 431
#define tDt 432
#define tDtDof 433
#define tDtDt 434
#define tDtDtDof 435
#define tDtDtDtDof 436
#define tDtDtDtDtDof 437
#define tDtDtDtDtDtDof 438
#define tNLEig1Dof 439
#define tNLEig2Dof 440
#define tNLEig3Dof 441
#define tNLEig4Dof 442
#define tNLEig5Dof 443
#define tNLEig6Dof 444
#define tJacNL 445
#define tDtDofJacNL 446
#define tNeverDt 447
#define tDtNL 448
#define tAtAnteriorTimeStep 449
#define tMaxOverTime 450
#define tFourierSteinmetz 451
#define tIn 452
#define tFull_Matrix 453
#define tResolution 454
#define tHidden 455
#define tDefineSystem 456
#define tNameOfFormulation 457
#define tNameOfMesh 458
#define tFrequency 459
#define tSolver 460
#define tOriginSystem 461
#define tDestinationSystem 462
#define tOperation 463
#define tOperationEnd 464
#define tSetTime 465
#define tSetTimeStep 466
#define tSetDTime 467
#define tDTime 468
#define tSetFrequency 469
#define tFourierTransform 470
#define tFourierTransformJ 471
#define tCopySolution 472
#define tCopyRHS 473
#define tCopyResidual 474
#define tCopyIncrement 475
#define tCopyDofs 476
#define tGetNormSolution 477
#define tGetNormResidual 478
#define tGetNormRHS 479
#define tGetNormIncrement 480
#define tLanczos 481
#define tEigenSolve 482
#define tEigenSolveJac 483
#define tPerturbation 484
#define tUpdate 485
#define tUpdateConstraint 486
#define tBreak 487
#define tGetResidual 488
#define tCreateSolution 489
#define tEvaluate 490
#define tSelectCorrection 491
#define tAddCorrection 492
#define tMultiplySolution 493
#define tAddOppositeFullSolution 494
#define tSolveAgainWithOther 495
#define tSetGlobalSolverOptions 496
#define tTimeLoopTheta 497
#define tTimeLoopNewmark 498
#define tTimeLoopRungeKutta 499
#define tTimeLoopAdaptive 500
#define tTime0 501
#define tTimeMax 502
#define tTheta 503
#define tBeta 504
#define tGamma 505
#define tIterativeLoop 506
#define tIterativeLoopN 507
#define tIterativeLinearSolver 508
#define tNbrMaxIteration 509
#define tRelaxationFactor 510
#define tIterativeTimeReduction 511
#define tSetCommSelf 512
#define tSetCommWorld 513
#define tBarrier 514
#define tBroadcastFields 515
#define tBroadcastVariables 516
#define tSleep 517
#define tDivisionCoefficient 518
#define tChangeOfState 519
#define tChangeOfCoordinates 520
#define tChangeOfCoordinates2 521
#define tSystemCommand 522
#define tError 523
#define tGmshRead 524
#define tGmshMerge 525
#define tGmshOpen 526
#define tGmshWrite 527
#define tGmshClearAll 528
#define tDelete 529
#define tDeleteFile 530
#define tRenameFile 531
#define tCreateDir 532
#define tGenerateOnly 533
#define tGenerateOnlyJac 534
#define tSolveJac_AdaptRelax 535
#define tSaveSolutionExtendedMH 536
#define tSaveSolutionMHtoTime 537
#define tSaveSolutionWithEntityNum 538
#define tInitMovingBand2D 539
#define tMeshMovingBand2D 540
#define tGenerateMHMoving 541
#define tGenerateMHMovingSeparate 542
#define tAddMHMoving 543
#define tGenerateGroup 544
#define tGenerateJacGroup 545
#define tGenerateRHSGroup 546
#define tGenerateGroupCumulative 547
#define tGenerateJacGroupCumulative 548
#define tGenerateRHSGroupCumulative 549
#define tSaveMesh 550
#define tDeformMesh 551
#define tFrequencySpectrum 552
#define tPostProcessing 553
#define tNameOfSystem 554
#define tPostOperation 555
#define tNameOfPostProcessing 556
#define tUsingPost 557
#define tResampleTime 558
#define tPlot 559
#define tPrint 560
#define tPrintGroup 561
#define tEcho 562
#define tSendMergeFileRequest 563
#define tWrite 564
#define tAdapt 565
#define tOnGlobal 566
#define tOnRegion 567
#define tOnElementsOf 568
#define tOnGrid 569
#define tOnSection 570
#define tOnPoint 571
#define tOnLine 572
#define tOnPlane 573
#define tOnBox 574
#define tWithArgument 575
#define tFile 576
#define tDepth 577
#define tDimension 578
#define tComma 579
#define tTimeStep 580
#define tHarmonicToTime 581
#define tCosineTransform 582
#define tTimeToHarmonic 583
#define tValueIndex 584
#define tValueName 585
#define tFormat 586
#define tHeader 587
#define tFooter 588
#define tSkin 589
#define tSmoothing 590
#define tTarget 591
#define tSort 592
#define tIso 593
#define tNoNewLine 594
#define tNoTitle 595
#define tDecomposeInSimplex 596
#define tChangeOfValues 597
#define tTimeLegend 598
#define tFrequencyLegend 599
#define tEigenvalueLegend 600
#define tEvaluationPoints 601
#define tStoreInRegister 602
#define tStoreInVariable 603
#define tStoreInField 604
#define tStoreInMeshBasedField 605
#define tStoreMaxInRegister 606
#define tStoreMaxXinRegister 607
#define tStoreMaxYinRegister 608
#define tStoreMaxZinRegister 609
#define tStoreMinInRegister 610
#define tStoreMinXinRegister 611
#define tStoreMinYinRegister 612
#define tStoreMinZinRegister 613
#define tLastTimeStepOnly 614
#define tAppendTimeStepToFileName 615
#define tTimeValue 616
#define tTimeImagValue 617
#define tTimeInterval 618
#define tAppendExpressionToFileName 619
#define tAppendExpressionFormat 620
#define tOverrideTimeStepValue 621
#define tNoMesh 622
#define tSendToServer 623
#define tDate 624
#define tOnelabAction 625
#define tCodeName 626
#define tFixRelativePath 627
#define tAppendToExistingFile 628
#define tAppendStringToFileName 629
#define tDEF 630
#define tOR 631
#define tAND 632
#define tAPPROXEQUAL 633
#define tNOTEQUAL 634
#define tEQUAL 635
#define tGREATERGREATER 636
#define tLESSLESS 637
#define tGREATEROREQUAL 638
#define tLESSOREQUAL 639
#define tCROSSPRODUCT 640
#define UNARYPREC 641
#define tSHOW 642




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 186 "ProParser.y"
{
  char    *c;
  int     i;
  double  d;
  List_T  *l;
  struct TwoInt t ;
  struct TwoChar c2;
}
/* Line 1529 of yacc.c.  */
#line 832 "ProParser.tab.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE getdp_yylval;


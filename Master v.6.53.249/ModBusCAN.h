//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloModBus.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			18/06/2011
//	Description:	Header delle funzioni che gestiscono la comunicazione ModBus con il sistema
//----------------------------------------------------------------------------------


#include "FWSelection.h"
#if (K_AbilMODBCAN==1)  

#ifndef _MODBUSCAN_H_
	#define _MODBUSCAN_H_

#include "ProtocolloComunicazione.h"
	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
    #define K_ModbCanSizeOfTxData                   5           // DLC frame 0x24 (Status) e 0x00 (Alive)
    #define K_ModbCanSizeOfRxData                   2           // DLC frame 0x26 (Command)
    #define K_ModbCanRoomNumber                     16          // Numero massimo di Room gestibili da HMI  

    #define	K_MODBCAN_ALIVE_CRCLO                   0xFF
    #define	K_MODBCAN_ALIVE_CRCHI                   0xFF

    #define K_StableValueChangeHMI                  3//3000        // Tempo in mS prima di accettare variazioni di valori provenienti da HMI

	// Registri lato Modbus del convertitore MODB-CAN ADFWEB

    // Registri comando ALIVE (in scrittura verso convertitore ADFWEB)
    // Modulo n.0
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD0           100
    #define	REG_MODBCAN_ALIVE_MINVER_MOD0           101
    #define	REG_MODBCAN_ALIVE_PATVER_MOD0           102
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD0            103
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD0            104     
    // Modulo n.1
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD1           105
    #define	REG_MODBCAN_ALIVE_MINVER_MOD1           106
    #define	REG_MODBCAN_ALIVE_PATVER_MOD1           107
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD1            108
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD1            109     
    // Modulo n.2
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD2           110
    #define	REG_MODBCAN_ALIVE_MINVER_MOD2           111
    #define	REG_MODBCAN_ALIVE_PATVER_MOD2           112
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD2            113
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD2            114     
    // Modulo n.3
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD3           115
    #define	REG_MODBCAN_ALIVE_MINVER_MOD3           116
    #define	REG_MODBCAN_ALIVE_PATVER_MOD3           117
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD3            118
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD3            119     
    // Modulo n.4
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD4           120
    #define	REG_MODBCAN_ALIVE_MINVER_MOD4           121
    #define	REG_MODBCAN_ALIVE_PATVER_MOD4           122
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD4            123
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD4            124
    // Modulo n.5
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD5           125
    #define	REG_MODBCAN_ALIVE_MINVER_MOD5           126
    #define	REG_MODBCAN_ALIVE_PATVER_MOD5           127
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD5            128
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD5            129
    // Modulo n.6
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD6           130
    #define	REG_MODBCAN_ALIVE_MINVER_MOD6           131
    #define	REG_MODBCAN_ALIVE_PATVER_MOD6           132
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD6            133
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD6            134
    // Modulo n.7
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD7           135
    #define	REG_MODBCAN_ALIVE_MINVER_MOD7           136
    #define	REG_MODBCAN_ALIVE_PATVER_MOD7           137
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD7            138
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD7            139
    // Modulo n.8
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD8           140
    #define	REG_MODBCAN_ALIVE_MINVER_MOD8           141
    #define	REG_MODBCAN_ALIVE_PATVER_MOD8           142
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD8            143
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD8            144
    // Modulo n.9
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD9           145
    #define	REG_MODBCAN_ALIVE_MINVER_MOD9           146
    #define	REG_MODBCAN_ALIVE_PATVER_MOD9           147
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD9            148
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD9            149
    // Modulo n.10
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD10          150
    #define	REG_MODBCAN_ALIVE_MINVER_MOD10          151
    #define	REG_MODBCAN_ALIVE_PATVER_MOD10          152
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD10           153
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD10           154
    // Modulo n.11
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD11          155
    #define	REG_MODBCAN_ALIVE_MINVER_MOD11          156
    #define	REG_MODBCAN_ALIVE_PATVER_MOD11          157
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD11           158
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD11           159
    // Modulo n.12
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD12          160
    #define	REG_MODBCAN_ALIVE_MINVER_MOD12          161
    #define	REG_MODBCAN_ALIVE_PATVER_MOD12          162
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD12           163
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD12           164
    // Modulo n.13
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD13          165
    #define	REG_MODBCAN_ALIVE_MINVER_MOD13          166
    #define	REG_MODBCAN_ALIVE_PATVER_MOD13          167
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD13           168
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD13           169
    // Modulo n.14
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD14          170
    #define	REG_MODBCAN_ALIVE_MINVER_MOD14          171
    #define	REG_MODBCAN_ALIVE_PATVER_MOD14          172
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD14           173
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD14           174
    // Modulo n.15
    #define	REG_MODBCAN_ALIVE_MAJVER_MOD15          175
    #define	REG_MODBCAN_ALIVE_MINVER_MOD15          176
    #define	REG_MODBCAN_ALIVE_PATVER_MOD15          177
    #define	REG_MODBCAN_ALIVE_CRCHI_MOD15           178
    #define	REG_MODBCAN_ALIVE_CRCLO_MOD15           179


	// Registri comando STATUS (in scrittura verso convertitore ADFWEB)
    // Modulo n.0
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD0		200
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD0		201
    #define	REG_MODBCAN_STATUS_TEMPER_MOD0			202
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD0			203
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD0			204
    // Modulo n.1
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD1		205
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD1		206
    #define	REG_MODBCAN_STATUS_TEMPER_MOD1			207
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD1			208
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD1			209
    // Modulo n.2
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD2		210
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD2		211
    #define	REG_MODBCAN_STATUS_TEMPER_MOD2			212
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD2			213
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD2			214
    // Modulo n.3
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD3		215
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD3		216
    #define	REG_MODBCAN_STATUS_TEMPER_MOD3			217
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD3			218
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD3			219
    // Modulo n.4
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD4		220
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD4		221
    #define	REG_MODBCAN_STATUS_TEMPER_MOD4			222
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD4			223
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD4			224
    // Modulo n.5
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD5		225
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD5		226
    #define	REG_MODBCAN_STATUS_TEMPER_MOD5			227
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD5			228
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD5			229
    // Modulo n.6
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD6		230
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD6		231
    #define	REG_MODBCAN_STATUS_TEMPER_MOD6			232
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD6			233
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD6			234
    // Modulo n.7
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD7		235
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD7		236
    #define	REG_MODBCAN_STATUS_TEMPER_MOD7			237
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD7			238
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD7			239
    // Modulo n.8
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD8		240
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD8		241
    #define	REG_MODBCAN_STATUS_TEMPER_MOD8			242
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD8			243
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD8			244
    // Modulo n.9
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD9		245
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD9		246
    #define	REG_MODBCAN_STATUS_TEMPER_MOD9			247
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD9			248
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD9			249
    // Modulo n.10
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD10		250
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD10		251
    #define	REG_MODBCAN_STATUS_TEMPER_MOD10			252
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD10		253
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD10		254
    // Modulo n.11
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD11		255
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD11		256
    #define	REG_MODBCAN_STATUS_TEMPER_MOD11			257
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD11		258
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD11		259
    // Modulo n.12
    #define	REG_MODBCAN_STATU2_MODE_FANS_MOD12		260
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD12		261
    #define	REG_MODBCAN_STATUS_TEMPER_MOD12			262
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD12		263
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD12		264
    // Modulo n.13
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD13		265
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD13		266
    #define	REG_MODBCAN_STATUS_TEMPER_MOD13			267
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD13		268
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD13		269
    // Modulo n.14
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD14		270
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD14		271
    #define	REG_MODBCAN_STATUS_TEMPER_MOD14			272
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD14		273
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD14		274
    // Modulo n.15
    #define	REG_MODBCAN_STATUS_MODE_FANS_MOD15		275
    #define	REG_MODBCAN_STATUS_AUX_SETP_MOD15		276
    #define	REG_MODBCAN_STATUS_TEMPER_MOD15			277
    #define	REG_MODBCAN_STATUS_SCHEDID_MOD15		278
    #define	REG_MODBCAN_STATUS_ERRCODE_MOD15		279


	// Registri comando COMMAND (in lettura da convertitore ADFWEB)
    // Modulo n.0
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD0		300
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD0		301
    // Modulo n.1
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD1		302
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD1		303
    // Modulo n.2
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD2		304
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD2		305
    // Modulo n.3
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD3		306
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD3		307
    // Modulo n.4
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD4		308
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD4		309
    // Modulo n.5
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD5		310
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD5		311
    // Modulo n.6
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD6		312
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD6		313
    // Modulo n.7
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD7		314
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD7		315
    // Modulo n.8
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD8		316
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD8		317
    // Modulo n.9
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD9		318
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD9		319
    // Modulo n.10
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD10		320
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD10		321
    // Modulo n.11
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD11		322
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD11		323
    // Modulo n.12
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD12		324
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD12		325
    // Modulo n.13
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD13		326
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD13		327
    // Modulo n.14
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD14		328
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD14		329
    // Modulo n.15
    #define	REG_MODBCAN_COMMAND_MODE_FANS_MOD15		330
    #define	REG_MODBCAN_COMMAND_AUX_SETP_MOD15		331


	// Registri comando TIME (in lettura da convertitore ADFWEB)
    #define	REG_MODBCAN_TIME_YEAR					340
    #define	REG_MODBCAN_TIME_MONTH					341
    #define	REG_MODBCAN_TIME_DAY					342
    #define	REG_MODBCAN_TIME_DAYWEEK				343
    #define	REG_MODBCAN_TIME_HOUR					344
    #define	REG_MODBCAN_TIME_MINUTE					345
    #define	REG_MODBCAN_TIME_SECOND					346



	// Registro comando SHUTDOWN (in lettura da convertitore ADFWEB)
    #define	REG_SCHEIBER_SHUTDOWN_COMMAND			350


    // Registri comando ALIVE (in lettura da convertitore ADFWEB)
    #define	REG_MODBCAN_ALIVE_MAJVER_HMI			360
    #define	REG_MODBCAN_ALIVE_MINVER_HMI			361
    #define	REG_MODBCAN_ALIVE_PATVER_HMI			362
    #define	REG_MODBCAN_ALIVE_CRCHI_HMI				363
    #define	REG_MODBCAN_ALIVE_CRCLO_HMI				364     



	// Registri comando STATUS da HMI (in lettura da convertitore ADFWEB)
    // Modulo n.0
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD0	400
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD0		401
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD0		402
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD0		403
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD0		404
    // Modulo n.1
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD1	405
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD1		406
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD1		207
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD1		208
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD1		209
    // Modulo n.2
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD2	410
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD2		411
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD2		412
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD2		413
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD2		414
    // Modulo n.3
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD3	415
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD3		416
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD3		417
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD3		418
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD3		419
    // Modulo n.4
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD4	420
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD4		421
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD4		422
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD4		423
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD4		424
    // Modulo n.5
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD5	425
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD5		426
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD5		427
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD5		428
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD5		429
    // Modulo n.6
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD6	430
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD6		431
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD6		432
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD6		433
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD6		434
    // Modulo n.7
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD7	435
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD7		436
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD7		437
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD7		438
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD7		439
    // Modulo n.8
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD8	440
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD8		441
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD8		442
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD8		443
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD8		444
    // Modulo n.9
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD9	445
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD9		446
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD9		447
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD9		448
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD9		449
    // Modulo n.10
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD10	450
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD10	451
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD10		452
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD10		453
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD10		454
    // Modulo n.11
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD11	455
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD11	456
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD11		457
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD11		458
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD11		459
    // Modulo n.12
    #define	REG_MODBCAN_STATU2_MODE_FANS_MOD12		460
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD12	461
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD12		462
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD12		463
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD12		464
    // Modulo n.13
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD13	465
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD13	466
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD13		467
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD13		468
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD13		469
    // Modulo n.14
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD14	470
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD14	471
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD14		472
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD14		473
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD14		474
    // Modulo n.15
    #define	REG_MODBCAN_STATUSHMI_MODE_FANS_MOD15	475
    #define	REG_MODBCAN_STATUSHMI_AUX_SETP_MOD15	476
    #define	REG_MODBCAN_STATUSHMI_TEMPER_MOD15		477
    #define	REG_MODBCAN_STATUSHMI_SCHEDID_MOD15		478
    #define	REG_MODBCAN_STATUSHMI_ERRCODE_MOD15		479



	// Registro di stato TIMEOUT (in lettura da convertitore ADFWEB)
    #define	REG_CAN_CONVERTER_TIMEOUT_FLAG			1000


	typedef struct
	{
		unsigned		Enable:1;					// se è abilitato
		unsigned		OffLine:1;					// se non ha risposto
		unsigned		OnLine:1;					// 
		unsigned		Ready:1;                    // 
        unsigned		InitializeModbCAN:1;		// 
		unsigned		PermanentOffLine:1;			// se non ha risposto entro "OffLineCounter"
        unsigned		CANBUS_Activity:1;          // flag di timeout convertitore ModbCAN (0 = attività lato CAN non rilevata)
        unsigned int    ChangedValueFromHMI;
		unsigned int	OffLineCounter;				// Numero di volte che non lo devo vedere Online per definirlo Offline
		unsigned int	ShotMaskFlag;               // Flag per mascherare a tempo l'invio dei cambiamenti "valori" agli slave solo quando il cambiamento da parte dell'utente è stabile
		unsigned int	ShotMaskCnt;                // Contatore per gestione Shot
        
		
		struct 
			{			
				int			 Power;				
				int			 DefMode;					
				int			 SetPoint;					
				int			 FanSpeed;				
				unsigned int TrigInit;						
			} Reinit;		

			struct 
			{			
				int			 PowerOn;				
				int			 DefMode;					
				int			 SetPoint;					
				int			 FanSpeed;				
				unsigned int SetHumi;		
			} SetUta;	

			struct 
			{			
				int			 Split;				
				int			 SubSplit;					
			} Diag;	

			struct 
			{			
				int         SystemDisable;			
                int         DefMode;
			} EngineBox;				
					
			struct 
			{			
				int			PowerOn;				
				int			DefMode;					
				int			SetPoint;					
				int			FanSpeed;				
				int         AirTemp;		
				int         CumulativeAlarm;
				int         AuxHeater;
			} Room[K_ModbCanRoomNumber+1];	            

			struct 
			{			
				int			PowerOn;				
				int			DefMode;					
				int			SetPoint;					
				int			FanSpeed;				
				int         AuxHeater;
				int         UpdateFlag;
				int         CntTrig;
                
                int         NewData0;
                int         NewData1;
                int         OldData0;
                int         OldData1;
			} SetRoom[K_ModbCanRoomNumber+1];	            
            
            
	} TypModbCAN;	    

	
	
	//---------------------------------------
	// Variabili 
	//---------------------------------------
	// Timer 
    
#define VariabiliGlobaliModBusCAN()             \
    volatile TypModbCAN ModbCAN;


#define IncludeVariabiliGlobaliModBusCAN()      \
    extern volatile TypModbCAN ModbCAN;    

    
    IncludeVariabiliGlobaliModBusCAN();     
	


	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------

	void Init_ModbCAN(void);
	int Check_IfModbCANIsPresent(void);
	void Send_InitDataToModbCAN(void);
	int Check_IfModbCANGoOnline(TypModbCAN * ModbCANflag);
	void Send_DataToModbCAN(void);	
	void Get_DataFromModbCAN(void);	
    void Send_AliveCANFrame_ToModbCAN(void);
    void PackDataForCAN(int RoomIndex);
    int CheckChangeValueRoomToHMI(int RoomIndex);
    int CheckChangeValueRoomFromHMI(int RoomIndex);
    void UnPackDataFromCAN(int RoomIndex);   
    void UpdateRoomDataFromCAN(int RoomIndex);
    void Send_UpdateDataToRoom(void);
    void RefreshDataForCAN(int RoomIndex);
    int LimitDataValue(int ValMin, int ValMax, int *Reg);
    void PackInitDataForCAN(int RoomIndex);
    
#endif
    
#endif //#if (K_AbilMODBCAN==1) 

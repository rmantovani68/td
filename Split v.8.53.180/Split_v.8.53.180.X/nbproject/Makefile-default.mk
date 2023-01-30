#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Split_v.8.53.180.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Split_v.8.53.180.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS
SUB_IMAGE_ADDRESS_COMMAND=--image-address $(SUB_IMAGE_ADDRESS)
else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../DefinePeriferiche.c ../Driver_Comunicazione.c ../I2C.c ../Main.c ../ProtocolloComunicazione.c ../Timer.c ../usart.c ../ADC.c ../EEPROM.c ../Valvola_PassoPasso.c ../PWM.c ../Driver_ComunicazioneSec.c ../ProtocolloComunicazioneSec.c ../Core.c ../PID.c ../LCD_ScreenInfo.c ../i2c_LCD_Driver.c ../soft_i2c.c ../ServiceComunication.c ../Driver_ModBusSec.c ../ProtocolloModBusSec.c ../Interrupt.c ../Driver_ModBus.c ../ProtocolloModBus.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1472/DefinePeriferiche.o ${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o ${OBJECTDIR}/_ext/1472/I2C.o ${OBJECTDIR}/_ext/1472/Main.o ${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o ${OBJECTDIR}/_ext/1472/Timer.o ${OBJECTDIR}/_ext/1472/usart.o ${OBJECTDIR}/_ext/1472/ADC.o ${OBJECTDIR}/_ext/1472/EEPROM.o ${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o ${OBJECTDIR}/_ext/1472/PWM.o ${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o ${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o ${OBJECTDIR}/_ext/1472/Core.o ${OBJECTDIR}/_ext/1472/PID.o ${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o ${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o ${OBJECTDIR}/_ext/1472/soft_i2c.o ${OBJECTDIR}/_ext/1472/ServiceComunication.o ${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o ${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o ${OBJECTDIR}/_ext/1472/Interrupt.o ${OBJECTDIR}/_ext/1472/Driver_ModBus.o ${OBJECTDIR}/_ext/1472/ProtocolloModBus.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1472/DefinePeriferiche.o.d ${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o.d ${OBJECTDIR}/_ext/1472/I2C.o.d ${OBJECTDIR}/_ext/1472/Main.o.d ${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o.d ${OBJECTDIR}/_ext/1472/Timer.o.d ${OBJECTDIR}/_ext/1472/usart.o.d ${OBJECTDIR}/_ext/1472/ADC.o.d ${OBJECTDIR}/_ext/1472/EEPROM.o.d ${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o.d ${OBJECTDIR}/_ext/1472/PWM.o.d ${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o.d ${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o.d ${OBJECTDIR}/_ext/1472/Core.o.d ${OBJECTDIR}/_ext/1472/PID.o.d ${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o.d ${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o.d ${OBJECTDIR}/_ext/1472/soft_i2c.o.d ${OBJECTDIR}/_ext/1472/ServiceComunication.o.d ${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o.d ${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o.d ${OBJECTDIR}/_ext/1472/Interrupt.o.d ${OBJECTDIR}/_ext/1472/Driver_ModBus.o.d ${OBJECTDIR}/_ext/1472/ProtocolloModBus.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1472/DefinePeriferiche.o ${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o ${OBJECTDIR}/_ext/1472/I2C.o ${OBJECTDIR}/_ext/1472/Main.o ${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o ${OBJECTDIR}/_ext/1472/Timer.o ${OBJECTDIR}/_ext/1472/usart.o ${OBJECTDIR}/_ext/1472/ADC.o ${OBJECTDIR}/_ext/1472/EEPROM.o ${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o ${OBJECTDIR}/_ext/1472/PWM.o ${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o ${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o ${OBJECTDIR}/_ext/1472/Core.o ${OBJECTDIR}/_ext/1472/PID.o ${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o ${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o ${OBJECTDIR}/_ext/1472/soft_i2c.o ${OBJECTDIR}/_ext/1472/ServiceComunication.o ${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o ${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o ${OBJECTDIR}/_ext/1472/Interrupt.o ${OBJECTDIR}/_ext/1472/Driver_ModBus.o ${OBJECTDIR}/_ext/1472/ProtocolloModBus.o

# Source Files
SOURCEFILES=../DefinePeriferiche.c ../Driver_Comunicazione.c ../I2C.c ../Main.c ../ProtocolloComunicazione.c ../Timer.c ../usart.c ../ADC.c ../EEPROM.c ../Valvola_PassoPasso.c ../PWM.c ../Driver_ComunicazioneSec.c ../ProtocolloComunicazioneSec.c ../Core.c ../PID.c ../LCD_ScreenInfo.c ../i2c_LCD_Driver.c ../soft_i2c.c ../ServiceComunication.c ../Driver_ModBusSec.c ../ProtocolloModBusSec.c ../Interrupt.c ../Driver_ModBus.c ../ProtocolloModBus.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

# The following macros may be used in the pre and post step lines
Device=PIC24FJ256GB210
ProjectDir="D:\Users\gluca\Dropbox\Condivisa Simone\LAST FW Master-Slave-Touch\Slave\Split v.8.53.180\Split_v.8.53.180.X"
ProjectName=Split_v.8.53.180
ConfName=default
ImagePath="dist\default\${IMAGE_TYPE}\Split_v.8.53.180.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ImageDir="dist\default\${IMAGE_TYPE}"
ImageName="Split_v.8.53.180.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IsDebug="true"
else
IsDebug="false"
endif

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/Split_v.8.53.180.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
	@echo "--------------------------------------"
	@echo "User defined post-build step: [wscript.exe WarningVersione.vbs]"
	@wscript.exe WarningVersione.vbs
	@echo "--------------------------------------"

MP_PROCESSOR_OPTION=24FJ256GB210
MP_LINKER_FILE_OPTION=,--script=p24FJ256GB210.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1472/DefinePeriferiche.o: ../DefinePeriferiche.c  .generated_files/7355efdbed364272601d33cfdc56c7d8d977f35d.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/DefinePeriferiche.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/DefinePeriferiche.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../DefinePeriferiche.c  -o ${OBJECTDIR}/_ext/1472/DefinePeriferiche.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/DefinePeriferiche.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o: ../Driver_Comunicazione.c  .generated_files/416bd6a2a37194c28938dad5959a6d4e80cf9796.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Driver_Comunicazione.c  -o ${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/I2C.o: ../I2C.c  .generated_files/98a9bfa2ad92ab19cd2b8c8fc28310710b163215.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/I2C.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/I2C.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../I2C.c  -o ${OBJECTDIR}/_ext/1472/I2C.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/I2C.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Main.o: ../Main.c  .generated_files/187beba13b57252584207e62bb7f0fecdd9d388a.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Main.c  -o ${OBJECTDIR}/_ext/1472/Main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Main.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o: ../ProtocolloComunicazione.c  .generated_files/53988904c0fdccc81c3a811ea248604e04629cc.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ProtocolloComunicazione.c  -o ${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Timer.o: ../Timer.c  .generated_files/9c68407b11af66fa183fb80999697b1c0eb750be.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Timer.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Timer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Timer.c  -o ${OBJECTDIR}/_ext/1472/Timer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Timer.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/usart.o: ../usart.c  .generated_files/c1b07eef065789df84c7089581213cca20f0859c.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usart.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../usart.c  -o ${OBJECTDIR}/_ext/1472/usart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/usart.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ADC.o: ../ADC.c  .generated_files/bb0404b2110d4cfd58bf0e2778cbe49ca3a90122.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ADC.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ADC.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ADC.c  -o ${OBJECTDIR}/_ext/1472/ADC.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ADC.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/EEPROM.o: ../EEPROM.c  .generated_files/d45f4663c3cb22a65add4c3306dfd5ca49efedbb.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/EEPROM.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/EEPROM.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../EEPROM.c  -o ${OBJECTDIR}/_ext/1472/EEPROM.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/EEPROM.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o: ../Valvola_PassoPasso.c  .generated_files/4930c6b9a7cac6fc452e5cd972e7d456d1f2e770.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Valvola_PassoPasso.c  -o ${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/PWM.o: ../PWM.c  .generated_files/7b9e6f7ed1739691189e89797f2508c9bd6696e5.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/PWM.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/PWM.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../PWM.c  -o ${OBJECTDIR}/_ext/1472/PWM.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/PWM.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o: ../Driver_ComunicazioneSec.c  .generated_files/a0f941f4d2917c363ca0f3dece6594ac706b90bf.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Driver_ComunicazioneSec.c  -o ${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o: ../ProtocolloComunicazioneSec.c  .generated_files/b86f5b7b21aa9b65ad0c8edbfd5c50402c00cfaf.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ProtocolloComunicazioneSec.c  -o ${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Core.o: ../Core.c  .generated_files/fe0f4199d5338e43e295135bec276e01e86cba84.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Core.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Core.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Core.c  -o ${OBJECTDIR}/_ext/1472/Core.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Core.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/PID.o: ../PID.c  .generated_files/6dae75b266471faa758b873b91d4d452c630602a.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/PID.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/PID.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../PID.c  -o ${OBJECTDIR}/_ext/1472/PID.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/PID.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o: ../LCD_ScreenInfo.c  .generated_files/a8120178e4d777d3a92f15dfb6bd437d682c172a.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../LCD_ScreenInfo.c  -o ${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o: ../i2c_LCD_Driver.c  .generated_files/b76130a0ead9c4d03178bd3c7849dca3db29e7b.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../i2c_LCD_Driver.c  -o ${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/soft_i2c.o: ../soft_i2c.c  .generated_files/e09f3f8d228229f99e4a22c002d5ea5fd7e2c10e.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/soft_i2c.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/soft_i2c.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../soft_i2c.c  -o ${OBJECTDIR}/_ext/1472/soft_i2c.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/soft_i2c.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ServiceComunication.o: ../ServiceComunication.c  .generated_files/7ce3acbfac0d910e7844a1d5a0033ad0c9e30270.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ServiceComunication.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ServiceComunication.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ServiceComunication.c  -o ${OBJECTDIR}/_ext/1472/ServiceComunication.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ServiceComunication.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o: ../Driver_ModBusSec.c  .generated_files/93886e11d4532b214eaf8feea42fa816992c95e2.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Driver_ModBusSec.c  -o ${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o: ../ProtocolloModBusSec.c  .generated_files/78267ebe193d55c2361a8a5f68343493a2ef4996.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ProtocolloModBusSec.c  -o ${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Interrupt.o: ../Interrupt.c  .generated_files/3873ee4a72a9f30c26fdb117387d4639cb2a070a.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Interrupt.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Interrupt.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Interrupt.c  -o ${OBJECTDIR}/_ext/1472/Interrupt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Interrupt.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Driver_ModBus.o: ../Driver_ModBus.c  .generated_files/4cc6a355a0da3019c1f06c78f4da9c19c1110d5c.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ModBus.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ModBus.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Driver_ModBus.c  -o ${OBJECTDIR}/_ext/1472/Driver_ModBus.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Driver_ModBus.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ProtocolloModBus.o: ../ProtocolloModBus.c  .generated_files/2ee06afbf75899d040892bb9ecd7ee19d36da7b4.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloModBus.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloModBus.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ProtocolloModBus.c  -o ${OBJECTDIR}/_ext/1472/ProtocolloModBus.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ProtocolloModBus.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1472/DefinePeriferiche.o: ../DefinePeriferiche.c  .generated_files/a24f1bbda56d8af62698f914feba579c7d1fb.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/DefinePeriferiche.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/DefinePeriferiche.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../DefinePeriferiche.c  -o ${OBJECTDIR}/_ext/1472/DefinePeriferiche.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/DefinePeriferiche.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o: ../Driver_Comunicazione.c  .generated_files/95ad277432eb8819def6423a675327eb9a505dc7.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Driver_Comunicazione.c  -o ${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Driver_Comunicazione.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/I2C.o: ../I2C.c  .generated_files/4a5ad66c360ea23ca4b7bce559648f60ff0948bf.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/I2C.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/I2C.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../I2C.c  -o ${OBJECTDIR}/_ext/1472/I2C.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/I2C.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Main.o: ../Main.c  .generated_files/a98379ea156751ba6c685d4c287e063296bb511f.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Main.c  -o ${OBJECTDIR}/_ext/1472/Main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o: ../ProtocolloComunicazione.c  .generated_files/e08450c8720f5c7b1d859fb415004aab665fea5d.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ProtocolloComunicazione.c  -o ${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ProtocolloComunicazione.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Timer.o: ../Timer.c  .generated_files/832f4a63f9ec6e991f2c433816a10fb7b7c6d8a0.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Timer.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Timer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Timer.c  -o ${OBJECTDIR}/_ext/1472/Timer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Timer.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/usart.o: ../usart.c  .generated_files/697722a0cd8df54bce3f0224b3adf3bd36571ab5.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usart.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../usart.c  -o ${OBJECTDIR}/_ext/1472/usart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/usart.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ADC.o: ../ADC.c  .generated_files/f717259c7ef9245157f36a6e7ccfd5794b6f47dd.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ADC.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ADC.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ADC.c  -o ${OBJECTDIR}/_ext/1472/ADC.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ADC.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/EEPROM.o: ../EEPROM.c  .generated_files/8491bc3fc79b18f1b8dbd2d9bd6b116b914f66ee.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/EEPROM.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/EEPROM.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../EEPROM.c  -o ${OBJECTDIR}/_ext/1472/EEPROM.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/EEPROM.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o: ../Valvola_PassoPasso.c  .generated_files/d377fc560ec29579663a9995419b28e5e52eea46.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Valvola_PassoPasso.c  -o ${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Valvola_PassoPasso.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/PWM.o: ../PWM.c  .generated_files/46ede4211983986394f21d05cc4b8b6a0c5d5dc2.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/PWM.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/PWM.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../PWM.c  -o ${OBJECTDIR}/_ext/1472/PWM.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/PWM.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o: ../Driver_ComunicazioneSec.c  .generated_files/b04acad7fcd3cf988e1b1a36b80dbf2e23a4dadb.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Driver_ComunicazioneSec.c  -o ${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Driver_ComunicazioneSec.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o: ../ProtocolloComunicazioneSec.c  .generated_files/b2c90aa807f4981f664450d58635e444432c49b5.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ProtocolloComunicazioneSec.c  -o ${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ProtocolloComunicazioneSec.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Core.o: ../Core.c  .generated_files/ddd3d8720423652d28346207d70d6fb907aac04e.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Core.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Core.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Core.c  -o ${OBJECTDIR}/_ext/1472/Core.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Core.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/PID.o: ../PID.c  .generated_files/969764bbc054c16b2baf4541bf2b5ad5ce821715.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/PID.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/PID.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../PID.c  -o ${OBJECTDIR}/_ext/1472/PID.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/PID.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o: ../LCD_ScreenInfo.c  .generated_files/54b690ec64276f5a624a2060d85210aeca457c6b.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../LCD_ScreenInfo.c  -o ${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/LCD_ScreenInfo.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o: ../i2c_LCD_Driver.c  .generated_files/42f3946c2502fc4e6d0820713f7ef33e038849a6.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../i2c_LCD_Driver.c  -o ${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/i2c_LCD_Driver.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/soft_i2c.o: ../soft_i2c.c  .generated_files/f2123b4585f7a8c8c5c3bf82e63c758ec341f0e1.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/soft_i2c.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/soft_i2c.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../soft_i2c.c  -o ${OBJECTDIR}/_ext/1472/soft_i2c.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/soft_i2c.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ServiceComunication.o: ../ServiceComunication.c  .generated_files/d8d3d67ba1cbf1800ee993ee0227e0818ce4c459.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ServiceComunication.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ServiceComunication.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ServiceComunication.c  -o ${OBJECTDIR}/_ext/1472/ServiceComunication.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ServiceComunication.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o: ../Driver_ModBusSec.c  .generated_files/c708f7469edbb8a7f5190c5c48e26570e29c5a52.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Driver_ModBusSec.c  -o ${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Driver_ModBusSec.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o: ../ProtocolloModBusSec.c  .generated_files/8aa7b9bfb6c0752463b06f59947403281956a86e.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ProtocolloModBusSec.c  -o ${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ProtocolloModBusSec.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Interrupt.o: ../Interrupt.c  .generated_files/6e3aa2719843f49f5535d034f81ff5ccfdd19fd6.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Interrupt.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Interrupt.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Interrupt.c  -o ${OBJECTDIR}/_ext/1472/Interrupt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Interrupt.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/Driver_ModBus.o: ../Driver_ModBus.c  .generated_files/ac1f7bfe190c5da7844200a98a1c80dfc17c48f6.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ModBus.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Driver_ModBus.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Driver_ModBus.c  -o ${OBJECTDIR}/_ext/1472/Driver_ModBus.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/Driver_ModBus.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1472/ProtocolloModBus.o: ../ProtocolloModBus.c  .generated_files/929e97a8f93ff11318fc4cd09f6846ddde180fbc.flag .generated_files/fec197a3b3c052cb255ae31f33f4f4748bb6512.flag
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloModBus.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProtocolloModBus.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../ProtocolloModBus.c  -o ${OBJECTDIR}/_ext/1472/ProtocolloModBus.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1472/ProtocolloModBus.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -mlarge-code -O0 -I".." -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/Split_v.8.53.180.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/Split_v.8.53.180.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG   -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x800:0x81B -mreserve=data@0x81C:0x81D -mreserve=data@0x81E:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x827 -mreserve=data@0x82A:0x84F   -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--library-path="..",--library-path=".",--no-force-link,--smart-io,-Map="${DISTDIR}/Split_v.8.4.22.X.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/Split_v.8.53.180.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/Split_v.8.53.180.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--library-path="..",--library-path=".",--no-force-link,--smart-io,-Map="${DISTDIR}/Split_v.8.4.22.X.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}\\xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/Split_v.8.53.180.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif

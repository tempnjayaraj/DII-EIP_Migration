@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this source code is subject to the terms of the Microsoft end-user
@REM license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
@REM If you did not accept the terms of the EULA, you are not authorized to use
@REM this source code. For a copy of the EULA, please see the LICENSE.RTF on your
@REM install media.
@REM
if /i not "%1"=="preproc" goto :Not_Preproc
    set CE_MODULES=
    set REPLACE_MODULES=
    set COREDLL_COMPONENTS=
    set CORELIBC_COMPONENTS=
    set NK_COMPONENTS=
    set PM_COMPONENTS=
    set FILESYS_COMPONENTS=
    set DEVICE_COMPONENTS=
    set COMMCTRL_COMPONENTS=
    set COREDLL_MESSAGEDIALOGBOXCUSTOMIZE_COMPONENT=
    set GWES_COMPONENTS=
    set GWE1_COMPONENTS=
    set GWE2_COMPONENTS=
    set GWE3_COMPONENTS=
    set GWE4_COMPONENTS=
    set FONTS_COMPONENTS=
    set PPP_COMPONENTS=
    set CRYPT32_COMPONENTS=
    set WINSOCK_COMPONENTS=
    set BTDRT_COMPONENTS=
    set BTD_COMPONENTS=
    set BTAGSVC_COMPONENTS=
    set FATUTIL_COMPONENTS=
    set WAVEAPI_COMPONENTS=
    set HTTPLITE_COMPONENTS=
    set NOTIFY_COMPONENTS=
    set REPLACE_COMPONENTS=
    set AFD_COMPONENTS=
    set ATAPI_COMPONENTS=
    set NWIFI_COMPONENTS=

    rem Clear our temporary variables
    set __SYSGEN 2>nul > %TEMP%\cebasetst.out
    for /f "tokens=1 delims==" %%f in (%TEMP%\cebasetst.out) do set %%f=
    goto :EOF
:Not_Preproc
if /i not "%1"=="pass1" goto :Not_Pass1
    REM
    REM Base OS components.
    REM
    set CE_MODULES=coredll nk
    set CORELIBC_COMPONENTS=ccrtstrt
    set COREDLL_COMPONENTS=coremain lmem showerr thunks corecrt corestrw

    REM =========================================================================================
    REM Equiv of old IABase config
    REM =========================================================================================
    if not "%SYSGEN_IABASE%"=="1" goto noIABase
        set __SYSGEN_FULLGWES=1
        set __SYSGEN_SHELL_APIS=1
        set SYSGEN_STDIOA=1
        set SYSGEN_CORESTRA=1
        set SYSGEN_FMTMSG=1
        set SYSGEN_TCPIP=1
        set SYSGEN_IPHLPAPI=1
        set SYSGEN_COMMCTRL=1
        set SYSGEN_COMMDLG=1
        set SYSGEN_IMM=1
        set __SYSGEN_NETUI=1
        set SYSGEN_PM=1
        set SYSGEN_FSDBASE=1
        set SYSGEN_NOTIFY=1
        set __SYSGEN_NEED_LOCUSA=1
        set SYSGEN_NLED=1
    :noIABase

    if "%__SYSGEN_STANSDK%"=="1" set CE_MODULES=%CE_MODULES% stansdk

    REM ==============================================================================================
    REM
    REM Platman components
    REM
    REM ==============================================================================================
    if "%SYSGEN_PLATMAN%"=="1" set CE_MODULES=%CE_MODULES% platman
    if "%SYSGEN_PLATMAN%"=="1" set SYSGEN_WINSOCK=1
    if "%SYSGEN_PLATMAN%"=="1" set SYSGEN_TOOLHELP=1


    REM ==============================================================================================
    REM
    REM IME components
    REM
    REM ==============================================================================================

    REM // Japanese IME 3.1 __________________________________________________________________________

    REM // Resolve component dependencies
    if "%SYSGEN_IMEJPN_ADVANCED_SETTING%" == "1" set SYSGEN_IMEJPN_PROPERTY=1
    if "%SYSGEN_IMEJPN_ADVANCED_SETTING%" == "1" set SYSGEN_CORESTRA=1
    if "%SYSGEN_IMEJPN_PROPERTY%" == "1" set SYSGEN_IMEJPN=1
    if "%SYSGEN_IMEJPN_DICTIONARY_TOOL%" == "1" set SYSGEN_IMEJPN=1
    if "%SYSGEN_IMEJPN_SYSTRAY%" == "1" set SYSGEN_IMEJPN=1
    if "%SYSGEN_IMEJPN_DB_STANDARD%" == "1" set SYSGEN_IMEJPN=1
    if "%SYSGEN_IMEJPN%" == "1" set SYSGEN_IMM=1
    if "%SYSGEN_IMEJPN%" == "1" set SYSGEN_CORELOC=1
    if "%SYSGEN_IMEJPN%" == "1" set SYSGEN_STDIOA=1
    if "%SYSGEN_IMEJPN%" == "1" set SYSGEN_STRSAFE=1
    if "%SYSGEN_IMEJPN%" == "1" set SYSGEN_COMMCTRL=1
    if "%SYSGEN_IMEJPN%" == "1" set __SYSGEN_FULLGWES=1
    if "%SYSGEN_IMEJPN%" == "1" set SYSGEN_CPP_EH_AND_RTTI=1

    if not "%SYSGEN_IMEJPN%" == "1" goto End_of_IMEJPN
        if "%SYSGEN_IMEJPN_DB_STANDARD%" == "" set SYSGEN_IMEJPN_DB_COMPACT=1
        REM // Core modules
        set CE_MODULES=%CE_MODULES% imjp31 imjp31k

        REM // Add dictionary
        if "%SYSGEN_IMEJPN_DB_STANDARD%" == "1" set CE_MODULES=%CE_MODULES% imjp31_dics_std
        if "%SYSGEN_IMEJPN_DB_COMPACT%" == "1" set CE_MODULES=%CE_MODULES% imjp31_dics_compact

        REM // Add optional components
        if "%SYSGEN_IMEJPN_SYSTRAY%" == "1" set CE_MODULES=%CE_MODULES% imjp31m
        if "%SYSGEN_QVGAP%"=="1" goto IMEJPN_Prop_and_Tools_QVGAP
            if "%SYSGEN_IMEJPN_PROPERTY%" == "1" set CE_MODULES=%CE_MODULES% imjp31u imjp31ux
            if "%SYSGEN_IMEJPN_DICTIONARY_TOOL%" == "1" set CE_MODULES=%CE_MODULES% imjp31dx
            if "%SYSGEN_IMEJPN_ADVANCED_SETTING%" == "1" set CE_MODULES=%CE_MODULES% imjp31c
            goto IMEJPN_Prop_and_Tools_end
        :IMEJPN_Prop_and_Tools_QVGAP
        if "%SYSGEN_IMEJPN_DICTIONARY_TOOL%" == "1" set CE_MODULES=%CE_MODULES% imjp31dx_q
        if "%SYSGEN_IMEJPN_PROPERTY%" == "1" set CE_MODULES=%CE_MODULES% imjp31u_q imjp31ux
        :IMEJPN_Prop_and_Tools_end

        REM // Select IME 3.1 skinnable UI component
        set IMEJPN_SKINNABLEUI_COMPONENT=imjpskin
        if "%SYSGEN_XPSKIN%" == "1" set IMEJPN_SKINNABLEUI_COMPONENT=imjpskin_xp
        if "%SYSGEN_PPC%" == "1" set IMEJPN_SKINNABLEUI_COMPONENT=imjpskin_ppc

        REM // Select IME 3.1 dictionary tool (portrait) component
        set IMEJPN_DICTIONARY_TOOL_PORTRAIT_COMPONENT=imjp31dx_q
        if "%__SYSGEN_IMEJPN_PPC%" == "1" set IMEJPN_DICTIONARY_TOOL_PORTRAIT_COMPONENT=imjp31dx_q_ppc

    :End_of_IMEJPN

    REM // Japanese Pocket IME 2.0 ___________________________________________________________________

    if "%SYSGEN_IMEJPN%" == "1" set SYSGEN_PIME=
    if "%SYSGEN_IMEJPN%" == "1" set SYSGEN_PIME_SUPPLEMENTAL_DATA=
    if "%SYSGEN_IMEJPN%" == "1" set SYSGEN_PIME_NAME_PLACE_DATA=

    REM // Resolve component dependencies
    if "%SYSGEN_PIME_SUPPLEMENTAL_DATA%" == "1" set SYSGEN_PIME=1
    if "%SYSGEN_PIME_NAME_PLACE_DATA%" == "1" set SYSGEN_PIME=1
    if "%SYSGEN_PIME%" == "1" set SYSGEN_IMM=1
    if "%SYSGEN_PIME%" == "1" set SYSGEN_CORELOC=1
    if "%SYSGEN_PIME%" == "1" set SYSGEN_COMMCTRL=1
    if "%SYSGEN_PIME%" == "1" set __SYSGEN_FULLGWES=1

    if "%SYSGEN_PIME%" == "1" set CE_MODULES=%CE_MODULES% imejpp imejppui
    if "%SYSGEN_PIME_SUPPLEMENTAL_DATA%" == "1" set CE_MODULES=%CE_MODULES% imejpp_dic_s
    if "%SYSGEN_PIME_NAME_PLACE_DATA%" == "1" set CE_MODULES=%CE_MODULES% imejpp_dic_n

    REM // TEST IME __________________________________________________________________________________

    REM // Resolve component dependencies
    if "%SYSGEN_TESTIME%" == "1" set SYSGEN_IMM=1
    if "%SYSGEN_TESTIME%" == "1" set SYSGEN_CORELOC=1
    if "%SYSGEN_TESTIME%" == "1" set __SYSGEN_FULLGWES=1
    if "%SYSGEN_TESTIME%" == "1" set SYSGEN_COMMCTRL=1

    if "%SYSGEN_TESTIME%" == "1" set CE_MODULES=%CE_MODULES% testime



    REM ==============================================================================================
    REM
    REM SIP and IM components
    REM
    REM ==============================================================================================

    REM // Resolve component dependencies
    if "%SYSGEN_CACJPN%" == "1" set __SYSGEN_IM_INCLUDED=1
    if "%SYSGEN_MULTIBOX%" == "1" set __SYSGEN_IM_INCLUDED=1
    if "%SYSGEN_IM_STROKE%"=="1"  set __SYSGEN_IM_INCLUDED=1
    if "%SYSGEN_IM_RADICAL%"=="1"  set __SYSGEN_IM_INCLUDED=1
    if "%SYSGEN_IM_ALLCHAR%"=="1"  set __SYSGEN_IM_INCLUDED=1
    if "%SYSGEN_IM_KANA%"=="1"  set __SYSGEN_IM_INCLUDED=1
    if "%SYSGEN_IM_ROMA%"=="1"  set __SYSGEN_IM_INCLUDED=1

    if "%__SYSGEN_IM_INCLUDED%" == "1" set __SYSGEN_FULLGWES=1
    if "%__SYSGEN_IM_INCLUDED%" == "1" set SYSGEN_IMM=1
    if "%__SYSGEN_IM_INCLUDED%" == "1" set SYSGEN_CORELOC=1
    if "%__SYSGEN_IM_INCLUDED%" == "1" set SYSGEN_STDIOA=1
    if "%__SYSGEN_IM_INCLUDED%" == "1" set SYSGEN_STRSAFE=1
    if "%__SYSGEN_IM_INCLUDED%" == "1" set SYSGEN_COMMCTRL=1
    if "%__SYSGEN_IM_INCLUDED%" == "1" set __SYSGEN_FULLGWES=1
    if "%__SYSGEN_IM_INCLUDED%" == "1" set SYSGEN_CPP_EH_AND_RTTI=1


    REM // CAC Japanese ______________________________________________________________________________

    if "%SYSGEN_CACJPN%" == "1" set CE_MODULES=%CE_MODULES% cacjpn
    if "%SYSGEN_CACJPN%" == "1" set SYSGEN_HWX=1

    REM // MULTIBOX __________________________________________________________________________________

    if "%SYSGEN_MULTIBOX%" == "1" set CE_MODULES=%CE_MODULES% multibox
    if "%SYSGEN_MULTIBOX%" == "1" set SYSGEN_HWX=1

    REM // JupiterJ's IMs ____________________________________________________________________________

    set __SYSGEN_IM_RADSTRK=0
    if "%SYSGEN_IM_RADICAL%"=="1"  set __SYSGEN_IM_RADSTRK=1
    if "%SYSGEN_IM_STROKE%"=="1"  set __SYSGEN_IM_RADSTRK=1
    if "%__SYSGEN_IM_RADSTRK%"=="1"  set CE_MODULES=%CE_MODULES% imeskdic
    if "%SYSGEN_IM_STROKE%"=="1"  set CE_MODULES=%CE_MODULES% msstrklist
    if "%SYSGEN_IM_RADICAL%"=="1"  set CE_MODULES=%CE_MODULES% msradlist
    if "%SYSGEN_IM_ALLCHAR%"=="1"  set CE_MODULES=%CE_MODULES% msallchar
    if "%SYSGEN_IM_KANA%"=="1"  set CE_MODULES=%CE_MODULES% mskana
    if "%SYSGEN_IM_ROMA%"=="1"  set CE_MODULES=%CE_MODULES% msroma


    REM ==============================================================================================
    REM
    REM CMD - Command Shell for Telnet and Console
    REM
    REM ==============================================================================================
    REM // Console support
    if not "%SYSGEN_CONSOLE%"=="1" goto noConsole
        set CE_MODULES=%CE_MODULES% console
        set SYSGEN_COMMCTRL=1
        set SYSGEN_CMD=1
        set __SYSGEN_FONTS_ANY_COUR=1
        set __SYSGEN_FULLGWES=1
    :noConsole

    REM CMD shell for telnet
    if not "%SYSGEN_CMD%"=="1" goto SkipCmd
        set CE_MODULES=%CE_MODULES% cmd
        set SYSGEN_STDIO=1
        set SYSGEN_FMTMSG=1
        set __SYSGEN_FILESYS=1
        set SYSGEN_FULL_CRT=1
        set __SYSGEN_NEED_LOCUSA=1
    :SkipCmd

    set __SYSGEN_GWE_CONTROLS=OS
    if "%SYSGEN_PPC%"=="1" set __SYSGEN_GWE_CONTROLS=PPC
    if "%__SYSGEN_TPC%"=="1" set __SYSGEN_GWE_CONTROLS=TPC



    REM // If you select an IM pull in the SIP
    if not "%SYSGEN_MSIM%"=="1" goto nomsim
       set SYSGEN_SOFTKB=1
       set CE_MODULES=%CE_MODULES% msim
       set SYSGEN_COMMCTRL=1
    :nomsim

    if not "%SYSGEN_TCHTEST%"=="1" goto noTchTest
        set CE_MODULES=%CE_MODULES% etcha inksamp inksamp2
        set SYSGEN_MINWMGR=1
        set SYSGEN_COMMCTRL=1
    :noTchTest

    REM
    if not "%SYSGEN_COMMCTRL%"=="1" goto noCommCtrl
        REM // Commctrl Logic
        set COMMCTRLLIB_RES=commctrl_hpc
        if "%SYSGEN_PPC%"=="1" set COMMCTRLLIB_RES=commctrl_ppc
        if "%__SYSGEN_TPC%"=="1" set COMMCTRLLIB_RES=commctrl_tpc


        REM //XP or 9X UI
        set COMMCTRL_SKIN=commctrlview
        if "%SYSGEN_XPSKIN%"=="1" set COMMCTRL_SKIN=commctrlviewxp


        set CE_MODULES=%CE_MODULES% commctrl
        set __SYSGEN_FULLGWES=1
        set __SYSGEN_SHELL_APIS=1
        set SYSGEN_GRADFILL=1
        set __SYSGEN_DSA=1
        set SYSGEN_STRSAFE=1

        REM // Commctrl components
        set COMMCTRL_COMPONENTS=toolbar updown status propsheet listview treeview date tab progress
        set COMMCTRL_COMPONENTS=%COMMCTRL_COMPONENTS% trackbar capedit rebar cmdbar dsa tooltips fe
        if "%SYSGEN_COMMCTRL_ANIMATE%"=="1" set COMMCTRL_COMPONENTS=%COMMCTRL_COMPONENTS% animate
        if "%__SYSGEN_COMMCTRL_LABELEDIT%"=="1" set COMMCTRL_COMPONENTS=%COMMCTRL_COMPONENTS% labeledit
        if "%__SYSGEN_COMMCTRL_BOXSELECT%"=="1" set COMMCTRL_COMPONENTS=%COMMCTRL_COMPONENTS% boxselect
        if "%__SYSGEN_COMMCTRL_SHAPIS%"=="1" set COMMCTRL_COMPONENTS=%COMMCTRL_COMPONENTS% shapis
    :noCommCtrl

    if not "%SYSGEN_COMMDLG%"=="1" goto noCommDlg
        set CE_MODULES=%CE_MODULES% commdlg
        set __SYSGEN_FULLGWES=1
    :noCommDlg


    REM =========================================================================================
    REM
    REM Debug Components (these should be removed by OEM prior to shipping)
    REM
    REM =========================================================================================
    REM Sample persistent registry and password
    if "%SYSGEN_OEMFS%"=="1" set CE_MODULES=%CE_MODULES% oemfs
    if "%SYSGEN_OEMFS%"=="1" set SYSGEN_SHELL=1

    REM Debug console, can also be run in cmd with "shell -c"
    if "%SYSGEN_SHELL%"=="1" set CE_MODULES=%CE_MODULES% shell relfsd loaddbg shellcelog
    if "%SYSGEN_SHELL%"=="1" set SYSGEN_STOREMGR=1
    if "%SYSGEN_SHELL%"=="1" set SYSGEN_FULL_CRT=1
    if "%SYSGEN_SHELL%"=="1" set SYSGEN_TOOLHELP=1

    if "%SYSGEN_LMEMDEBUG%"=="1" set CE_MODULES=%CE_MODULES% lmemdebug
    if "%SYSGEN_LMEMDEBUG%"=="1" set __SYSGEN_FILESYS=1
    if "%SYSGEN_LMEMDEBUG%"=="1" set SYSGEN_TOOLHELP=1

    if "%SYSGEN_CERDISP%"=="1" set CE_MODULES=%CE_MODULES% cerdisp
    if "%SYSGEN_CERDISP%"=="1" set SYSGEN_WINSOCK=1
    if "%SYSGEN_CERDISP%"=="1" set __SYSGEN_FULLGWES=1

    if not "%SYSGEN_USREXCEPTDMP%"=="1" goto noUsrExeptDmp
        set CE_MODULES=%CE_MODULES% UsrExceptDmp
        set SYSGEN_STRSAFE=1
        set __SYSGEN_FILESYS=1
    :noUsrExeptDmp

    if "%SYSGEN_WATSON_DMPGEN%"=="1" set CE_MODULES=%CE_MODULES% osaxst0

    REM ==============================================================================================
    REM
    REM WCETK - Test Kit components
    REM
    REM ==============================================================================================
    if "%SYSGEN_WCETK%"=="1" set CE_MODULES=%CE_MODULES% wcetk

    REM Bluetooth
    if "%SYSGEN_BTH_HID_KEYBOARD%"=="1" set __SYSGEN_HID_KEYBOARD=1
    if "%SYSGEN_BTH_HID_KEYBOARD%"=="1" set __SYSGEN_BTH_HID=1
    if "%SYSGEN_BTH_HID_MOUSE%"=="1"    set __SYSGEN_HID_MOUSE=1
    if "%SYSGEN_BTH_HID_MOUSE%"=="1"    set __SYSGEN_BTH_HID=1

    if "%SYSGEN_BTH_UART_ONLY%"=="1" set __SYSGEN_BTH=1
    if "%SYSGEN_BTH_CSR_ONLY%"=="1" set __SYSGEN_BTH=1
    if "%SYSGEN_BTH_USB_ONLY%"=="1" set __SYSGEN_BTH=1
    if "%SYSGEN_BTH_USB_ONLY%"=="1" set SYSGEN_USB=1
    if "%SYSGEN_BTH_SDIO_ONLY%"=="1" set __SYSGEN_BTH=1
    if "%SYSGEN_BTH_SDIO_ONLY%"=="1" set SYSGEN_SDBUS=1

    if "%SYSGEN_BTH_PAN%"=="1" set SYSGEN_NDIS=1
    if "%SYSGEN_BTH_PAN%"=="1" set __SYSGEN_BTH=1

    REM if "%SYSGEN_BTH_A2DP%"=="1" set SYSGEN_BTH_AVDTP=1
    REM if "%SYSGEN_BTH_AVDTP%"=="1" set __SYSGEN_BTH=1
    REM if "%SYSGEN_BTH_AVRCP%"=="1" set SYSGEN_BTH_AVCTP=1
    REM if "%SYSGEN_BTH_AVCTP%"=="1" set __SYSGEN_BTH=1

    if "%SYSGEN_BTH_MODEM%"=="1" set __SYSGEN_BTH=1
    if "%SYSGEN_BTH_AG%"=="1" set __SYSGEN_BTH=1
    if "%SYSGEN_BTH_AG%"=="1" set SYSGEN_WINSOCK=1
    if "%SYSGEN_BTH_AG%"=="1" set SYSGEN_TAPI=1
    if "%SYSGEN_BTH_AUDIO%"=="1" set __SYSGEN_BTH=1
    if "%__SYSGEN_BTH_HID%"=="1" set __SYSGEN_BTH=1

    if "%SYSGEN_BTH_UTILS%"=="1" set __SYSGEN_BTH=1

    if "%SYSGEN_BTH%"=="1" set __SYSGEN_BTH=1
    if "%__SYSGEN_BTH%"=="1" set __SYSGEN_NEED_LOCUSA=1
    if "%__SYSGEN_BTH%"=="1" set SYSGEN_STDIO=1
    if "%__SYSGEN_BTH%"=="1" set SYSGEN_STDIOA=1
    if "%__SYSGEN_BTH%"=="1" set SYSGEN_DEVICE=1

    if "%__SYSGEN_LPC%"=="1" set CE_MODULES=%CE_MODULES% lpcd lpcrt
    if "%__SYSGEN_LPC%"=="1" set SYSGEN_DEVICE=1

    REM // SCARD (Smart Card Resource Manager APIs) & Smartcard reader drivers
    REM SCM Microsystems PCMCIA reader
    if "%SYSGEN_SMARTCARD_PCMCIA%"=="1" set CE_MODULES=%CE_MODULES% pscr
    if "%SYSGEN_SMARTCARD_PCMCIA%"=="1" set SYSGEN_SMARTCARD=1
    REM // BULL Systems SmarTLP3 Serial reader
    if "%SYSGEN_SMARTCARD_SERIAL%"=="1" set CE_MODULES=%CE_MODULES% bulltlp3
    if "%SYSGEN_SMARTCARD_SERIAL%"=="1" set SYSGEN_SMARTCARD=1
    if "%SYSGEN_USB_SMARTCARD%"=="1"    set SYSGEN_USB=1
    if "%SYSGEN_USB_SMARTCARD%"=="1"    set SYSGEN_SMARTCARD=1

    if "%SYSGEN_SMARTCARD%"=="1" set CE_MODULES=%CE_MODULES% scard winscard
    if "%SYSGEN_SMARTCARD%"=="1" set __SYSGEN_NEED_LOCUSA=1

    if "%SYSGEN_L2TP%"=="1"       set SYSGEN_IPHLPAPI=1
    if "%SYSGEN_L2TP%"=="1"       set SYSGEN_PPP=1
    if "%SYSGEN_L2TP%"=="1"       set SYSGEN_IPSEC=1

    if not "%SYSGEN_IPSEC%" == "1" goto noipsec
        set SYSGEN_CERTS=1
        set SYSGEN_CRYPTO_DSSDH=1
	set SYSGEN_DEVICE=1
	set SYSGEN_STDIOA=1
        set __SYSGEN_FILESYS=1
        set __SYSGEN_NEED_LOCUSA=1
	set SYSGEN_FMTMSG=1
    :noipsec

    REM ==============================================================================================
    REM
    REM Optional Security Components (CAPI, SSPI, NTLM, Kerberos)
    REM
    REM ==============================================================================================

    REM PPP Server uses NTLMSSP support for auth
    if "%SYSGEN_PPP_SERVER%"=="1" set SYSGEN_AUTH_NTLM=1

    REM Secure Dynamic DNS uses kerberos based authentication
    if "%SYSGEN_SECURE_DDNS%"=="1" set SYSGEN_DNSAPI=1
    if "%SYSGEN_SECURE_DDNS%"=="1" set SYSGEN_AUTH_KERBEROS=1

    REM // SSPI
    if "%SYSGEN_AUTH_NTLM%"=="1"     set SYSGEN_AUTH=1
    if "%SYSGEN_AUTH_KERBEROS%"=="1" set SYSGEN_AUTH=1
    if "%SYSGEN_AUTH_SCHANNEL%"=="1" set SYSGEN_AUTH=1
    REM Required for credential manager
    if "%SYSGEN_REDIR%"=="1"         set SYSGEN_AUTH=1
    if "%SYSGEN_REDIR_ONLY%"=="1" goto no_spnego
        if "%SYSGEN_REDIR%"=="1" if "%SYSGEN_AUTH_NTLM%"=="1" set __SYSGEN_AUTH_SPNEGO=1
    :no_spnego

    REM If both redir and cmd are specified then include net.exe
    if "%SYSGEN_REDIR%"=="1" if "%SYSGEN_CMD%"=="1" set CE_MODULES=%CE_MODULES% net

    if not "%SYSGEN_AUTH%"=="1" goto skipauth
        set CE_MODULES=%CE_MODULES% secur32
    set SYSGEN_CREDMAN=1
        set __SYSGEN_FILESYS=1
        set __SYSGEN_NEED_LOCUSA=1
        REM // Secure components: SSPI, ntlm SSP, kerberos SSP;
        if "%SYSGEN_AUTH_NTLM%"=="1" set CE_MODULES=%CE_MODULES% ntlmssp
        if "%SYSGEN_AUTH_KERBEROS%"=="1" set __SYSGEN_AUTH_SPNEGO=1
        if "%SYSGEN_AUTH_KERBEROS%"=="1" set __SYSGEN_MSASN1=1
        if "%SYSGEN_AUTH_KERBEROS%"=="1" set CE_MODULES=%CE_MODULES% kerberos cryptdll
        if "%__SYSGEN_AUTH_SPNEGO%" == "1" set __SYSGEN_MSASN1=1
        if "%__SYSGEN_AUTH_SPNEGO%" == "1" set CE_MODULES=%CE_MODULES% spnego
        if "%SYSGEN_AUTH_KERBEROS%"=="1" set SYSGEN_TCPIP=1
        if "%SYSGEN_AUTH_KERBEROS%"=="1" set SYSGEN_FULL_CRT=1
        if not "%SYSGEN_AUTH_SCHANNEL%"=="1" goto skipschannel
        REM // Secure channel SSL provider (now depends on crypt32)
            set CE_MODULES=%CE_MODULES% schannel
            set SYSGEN_CRYPTO=1
            set SYSGEN_CERTS=1
            set SYSGEN_TCPIP=1
            set WINSOCK_COMPONENTS=sslsock
        :skipschannel
    :skipauth

    REM // CAPI
    if "%SYSGEN_CRYPTMSG%"=="1"         set SYSGEN_CERTS=1
    if "%SYSGEN_CERTS_PFX%"=="1"        set SYSGEN_CERTS=1
    if "%SYSGEN_CERTS%" == "1"          set SYSGEN_CRYPTO=1
    if "%SYSGEN_NWIFI_AP%"=="1"         set SYSGEN_CRYPTO=1
    if "%SYSGEN_NWIFI_STA%"=="1"        set SYSGEN_CRYPTO=1
    if "%SYSGEN_CRYPTO_SCWCSP%" == "1"  set SYSGEN_CRYPTO=1
    if "%SYSGEN_CRYPTO_SCWCSP%" == "1"  set SYSGEN_MINWMGR=1
    if "%SYSGEN_CRYPTO_DSSDH%" == "1"  set SYSGEN_CRYPTO=1

    if not "%SYSGEN_CRYPTO%"=="1" goto skipcrypto
    REM // do not set __SYSGEN_BUILDCSPS after CAPI signature verification is on
        REM set __SYSGEN_BUILDCSPS=1

        set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% cryptapi 
        set __SYSGEN_FILESYS=1
        REM // RSAENH crypto service provider is included by default
        if "%REPLACE_SYSGEN_RSAENH%"=="1" goto no_rsaenh
            set CE_MODULES=%CE_MODULES% rsaenh
        :no_rsaenh
        if "%SYSGEN_SMARTCARD%" == "1" if "%SYSGEN_CRYPTO_SCWCSP%" == "1" set CE_MODULES=%CE_MODULES%  scwapi scwcsp scwcspgui
        REM // Diffie-Hellman CSP
        if "%REPLACE_SYSGEN_DSSDH%"=="1" goto no_dssdh
            if "%SYSGEN_CRYPTO_DSSDH%" == "1" set CE_MODULES=%CE_MODULES% dssdh
        :no_dssdh


        REM // CAPI2
        if "%SYSGEN_CERTS%" == "1" set __SYSGEN_MSASN1=1
        if "%SYSGEN_CERTS%" == "1" set CE_MODULES=%CE_MODULES% crypt32
        if "%SYSGEN_CRYPTMSG%" == "1" set CRYPT32_COMPONENTS=%CRYPT32_COMPONENTS% wincrmsg
        if "%SYSGEN_CERTS_PFX%" == "1" set CRYPT32_COMPONENTS=%CRYPT32_COMPONENTS% pfx

        if "%SYSGEN_CERTS%" == "1" set SYSGEN_CORESTRA=1

        if "%SYSGEN_ENROLL%" == "1" set CE_MODULES=%CE_MODULES% enroll
        if "%SYSGEN_ENROLL%" == "1" set SYSGEN_STDIOA=1
    :skipcrypto
    REM //ASN.1 dll used by crypto/auth modules
    if "%__SYSGEN_MSASN1%"=="1" set CE_MODULES=%CE_MODULES% msasn1
    if "%__SYSGEN_MSASN1%"=="1" set SYSGEN_CORESTRA=1

    REM Credential Manager
    if not "%SYSGEN_CREDMAN%"=="1" goto skipCredman
    set __SYSGEN_FILESYS=1
    set CE_MODULES=%CE_MODULES% credman
    :skipCredman

    REM Password Local Authentication Plugin.
    REM (Needs LASS to Load, Check/SetPassword and UI Functionality.
    if "%SYSGEN_LAP_PSWD%"=="1" set SYSGEN_LASS=1
    if "%SYSGEN_LAP_PSWD%"=="1" set SYSGEN_FSPASSWORD=1
    if "%SYSGEN_LAP_PSWD%"=="1" set SYSGEN_MINWMGR=1

    REM Local Authentication Sub System Needs to read the registry
    if "%SYSGEN_LASS%"=="1" set  __SYSGEN_FILESYS=1

    REM // SD client driver dependencies (Note: Bluetooth client located elsewhere)
    if "%SYSGEN_SD_MEMORY%"=="1"        set SYSGEN_SDBUS=1
    if "%SYSGEN_SD_MEMORY%"=="1"        set SYSGEN_FATFS=1
    if "%SYSGEN_SD_MEMORY%"=="1"        set SYSGEN_STOREMGR=1
    if "%SYSGEN_SD_MEMORY%"=="1"        set SYSGEN_CORESTRA=1

    REM // SD dependencies
    if "%SYSGEN_SDHC_STANDARD%"=="1"    set SYSGEN_SDBUS=1
    if "%SYSGEN_SDBUS%"=="1"            set SYSGEN_DEVICE=1

    REM // USB dependencies
    if "%SYSGEN_USB_HID_CLIENTS%"=="1"  set SYSGEN_USB_HID_KEYBOARD=1
    if "%SYSGEN_USB_HID_CLIENTS%"=="1"  set SYSGEN_USB_HID_MOUSE=1
    if "%SYSGEN_USB_HID_KEYBOARD%"=="1" set SYSGEN_USB_HID=1
    if "%SYSGEN_USB_HID_KEYBOARD%"=="1" set __SYSGEN_HID_KEYBOARD=1
    if "%SYSGEN_USB_HID_MOUSE%"=="1"    set SYSGEN_USB_HID=1
    if "%SYSGEN_USB_HID_MOUSE%"=="1"    set __SYSGEN_HID_MOUSE=1
    if "%SYSGEN_USB_HID%"=="1"          set SYSGEN_USB=1
    if "%SYSGEN_USB_HID%"=="1"          set __SYSGEN_HID_PARSER=1
    if "%SYSGEN_USB_HID%"=="1"          set __SYSGEN_NEED_PMSTUBS=1
    if "%SYSGEN_USB_PRINTER%"=="1"      set SYSGEN_USB=1
    if "%SYSGEN_USB_STORAGE%"=="1"      set SYSGEN_USB=1
    if "%SYSGEN_USB_STORAGE%"=="1"      set SYSGEN_STOREMGR=1
    if "%SYSGEN_USB_STORAGE%"=="1"      set SYSGEN_CORESTRA=1
    if "%SYSGEN_ETH_USB_HOST%"=="1"     set SYSGEN_USB=1
    if "%SYSGEN_ETH_USB_HOST%"=="1"     set SYSGEN_ETHERNET=1

    REM // HID dependencies
    if "%__SYSGEN_HID_KEYBOARD%"=="1"   set __SYSGEN_HID_PARSER=1
    if "%__SYSGEN_HID_KEYBOARD%"=="1"   set SYSGEN_MINWMGR=1
    if "%__SYSGEN_HID_MOUSE%"=="1"      set __SYSGEN_HID_PARSER=1
    if "%__SYSGEN_HID_MOUSE%"=="1"      set SYSGEN_MINWMGR=1
    if "%__SYSGEN_HID_PARSER%"=="1"     set SYSGEN_CORESTRA=1

    REM // USB Function dependencies
    if "%SYSGEN_USBFN_NET2280%"=="1"    set SYSGEN_USBFN=1
    if "%SYSGEN_USBFN_STORAGE%"=="1"    set SYSGEN_USBFN=1
    if "%SYSGEN_USBFN_STORAGE%"=="1"    set SYSGEN_STOREMGR=1
    if "%SYSGEN_USBFN_ETHERNET%"=="1"   set SYSGEN_USBFN=1
    if "%SYSGEN_USBFN_ETHERNET%"=="1"   set SYSGEN_ETHERNET=1
    if "%SYSGEN_USBFN_SERIAL%"=="1"     set SYSGEN_USBFN=1
    if "%SYSGEN_USBFN%"=="1"            set SYSGEN_DEVICE=1

    REM // Parallel support
    if "%SYSGEN_PARALLEL%"=="1" set CE_MODULES=%CE_MODULES% parallel

    REM // Printing support
    if "%SYSGEN_PCL%"=="1" set CE_MODULES=%CE_MODULES% pcl
    if "%SYSGEN_PCL%"=="1" set SYSGEN_PRINTING=1

    if "%SYSGEN_PRINTING%"=="1" set CE_MODULES=%CE_MODULES% prnport prnerr
    if "%SYSGEN_PRINTING%"=="1" set SYSGEN_WINSOCK=1

    REM ==============================================================================================
    REM
    REM Uniscribe module to enable Complex scripting support
    REM
    REM ==============================================================================================
    if not "%SYSGEN_UNISCRIBE%"=="1" goto no_uniscribe
        set CE_MODULES=%CE_MODULES% uspce
        set SYSGEN_CORELOC=1
        set SYSGEN_MINGDI=1
        set SYSGEN_MINWMGR=1
        set SYSGEN_FMTMSG=1
        set __SYSGEN_GWE_MGTCI=1
    :no_uniscribe

    REM ==============================================================================================
    REM
    REM Optional Notification Components
    REM
    REM ==============================================================================================
    if "%SYSGEN_TIMESVC_DST%"=="1"  if not "%SYSGEN_NOTIFY%"=="1" set SYSGEN_MINNOTIFY=1

    if "%SYSGEN_NOTIFY%"=="1" set SYSGEN_MINNOTIFY=

    if "%SYSGEN_MINNOTIFY%"=="1" set SYSGEN_FSDBASE=1
    if "%SYSGEN_MINNOTIFY%"=="1" set SYSGEN_DEVICE=1
    if "%SYSGEN_MINNOTIFY%"=="1" set CE_MODULES=%CE_MODULES% notify
    if "%SYSGEN_MINNOTIFY%"=="1" set NOTIFY_COMPONENTS=notifmin notifnoui
    if "%SYSGEN_MINNOTIFY%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% tnotify

    if "%SYSGEN_NOTIFY%"=="1" set SYSGEN_FSDBASE=1
    if "%SYSGEN_NOTIFY%"=="1" set SYSGEN_DEVICE=1
    if "%SYSGEN_NOTIFY%"=="1" set __SYSGEN_FULLGWES=1
    if "%REPLACE_SYSGEN_NOTIFY%"=="1" goto no_notify_module
        if "%SYSGEN_NOTIFY%"=="1" set CE_MODULES=%CE_MODULES% notify
    :no_notify_module
    if "%SYSGEN_NOTIFY%"=="1" set NOTIFY_COMPONENTS=notifpub notifui
    if "%SYSGEN_NOTIFY%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% tnotify

    if "%NOTIFY_COMPONENTS%"=="" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% snotify

    REM ==============================================================================================
    REM
    REM Optional Networking Components
    REM
    REM ==============================================================================================

    REM If both redirector and standard shell are specified then include netui
    if "%SYSGEN_REDIR%"=="1" if "%SYSGEN_STANDARDSHELL%"=="1" set __SYSGEN_NETUI=1

    REM Network connections UI
    if "%SYSGEN_CONNMC%"=="1" set CE_MODULES=%CE_MODULES% rnaapp connmc ndispwr
    if "%SYSGEN_CONNMC%"=="1" set __SYSGEN_NETUI=1

    REM Sample UI for networking/driver modules
    if not "%__SYSGEN_NETUI%"=="1" goto noNetUI
        set CE_MODULES=%CE_MODULES% netui
        set __SYSGEN_FILESYS=1
        set SYSGEN_WINSOCK=1
        set SYSGEN_MINWMGR=1
        set SYSGEN_FMTMSG=1
        set SYSGEN_IPHLPAPI=1
        set __SYSGEN_SHELL_APIS=1
    :noNetUI

    if not "%SYSGEN_NETAPI32%"=="1" goto noNetApi32
        set CE_MODULES=%CE_MODULES% netapi32
        set SYSGEN_LDAP=1
        set SYSGEN_DNSAPI=1
    :noNetApi32

    REM // LDAP
    if "%SYSGEN_LDAP%"=="1" set SYSGEN_WINSOCK=1
    if "%SYSGEN_LDAP%"=="1" set CE_MODULES=%CE_MODULES% wldap32

    REM // HTTP Lite
    if "%__SYSGEN_NEED_HTTP%"=="1" if not "%SYSGEN_WININET%"=="1" set __SYSGEN_HTTPLITE=1
    if "%__SYSGEN_NEED_HTTP_FTP%"=="1" if not "%SYSGEN_WININET%"=="1" set __SYSGEN_HTTPLITE_FTP=1

    if "%__SYSGEN_HTTPLITE_FTP%"=="1" set __SYSGEN_HTTPLITE=1
    if "%__SYSGEN_HTTPLITE%"=="1"     set CE_MODULES=%CE_MODULES% httplite
    if "%__SYSGEN_HTTPLITE%"=="1"     set SYSGEN_CORESTRA=1
    if "%__SYSGEN_HTTPLITE%"=="1"     set SYSGEN_FULL_CRT=1
    if "%__SYSGEN_HTTPLITE%"=="1"     set SYSGEN_TCPIP=1
    if "%__SYSGEN_HTTPLITE%"=="1"     set SYSGEN_WINSOCK=1
    if "%__SYSGEN_HTTPLITE%"=="1"     set __SYSGEN_NEED_LOCUSA=1
    if "%__SYSGEN_HTTPLITE_FTP%"=="1" set HTTPLITE_COMPONENTS=httpftp

    REM Standard modem support for dial up networking and DCC.  Minimal
    REM configs with built in modem can remove asyncmac and/or unimodem.
    if "%SYSGEN_MODEM%"=="1"      set SYSGEN_ASYNCMAC=1
    if "%SYSGEN_MODEM%"=="1"      set SYSGEN_UNIMODEM=1
    if "%SYSGEN_ASYNCMAC%"=="1"   set SYSGEN_PPP=1
    if "%SYSGEN_ASYNCMAC%"=="1"   set SYSGEN_TAPI=1
    if "%SYSGEN_ASYNCMAC%"=="1"   set CE_MODULES=%CE_MODULES% asyncmac
    if "%SYSGEN_UNIMODEM%"=="1"   set SYSGEN_TAPI=1
    if "%SYSGEN_UNIMODEM%"=="1"   set __SYSGEN_NEED_LOCUSA=1
    if "%SYSGEN_UNIMODEM%"=="1"   set CE_MODULES=%CE_MODULES% unimodem

    if "%SYSGEN_ETH_80211%"=="1"  set SYSGEN_ETHERNET=1
    if "%SYSGEN_NWIFI_AP%"=="1"   set SYSGEN_ETHERNET=1
    if "%SYSGEN_NWIFI_STA%"=="1"  set SYSGEN_ETHERNET=1

    if "%SYSGEN_NETUTILS%"=="1"   set SYSGEN_IPHLPAPI=1
    if "%SYSGEN_SNMP%"=="1"       set SYSGEN_IPHLPAPI=1
    if "%SYSGEN_ETH_80211%"=="1"  set SYSGEN_IPHLPAPI=1
    if "%SYSGEN_TCPIP6%"=="1"     set SYSGEN_IPHLPAPI=1
    if "%SYSGEN_AUTORAS%"=="1"    set SYSGEN_PPP=1
    if "%SYSGEN_PPTP%"=="1"       set SYSGEN_IPHLPAPI=1
    if "%SYSGEN_PPTP%"=="1"       set SYSGEN_PPP=1
    if "%SYSGEN_PPPOE%"=="1"      set SYSGEN_PPP=1
    if "%SYSGEN_PPP_SERVER%"=="1" set SYSGEN_PPP=1
    if "%SYSGEN_PPP%"=="1"        set SYSGEN_IPHLPAPI=1

    REM Most net components require TCP/IP, winsock, NDIS
    if "%SYSGEN_ETHERNET%"=="1"   set SYSGEN_TCPIP=1
    if "%SYSGEN_ETH_OTHER%"=="1"  set SYSGEN_TCPIP=1
    if "%SYSGEN_GATEWAY%"=="1"    set SYSGEN_TCPIP=1
    if "%SYSGEN_BRIDGE%"=="1"     set SYSGEN_TCPIP=1
    if "%SYSGEN_BRIDGE2%"=="1"    set SYSGEN_TCPIP=1
    if "%SYSGEN_PPP%"=="1"        set SYSGEN_TCPIP=1
    if "%SYSGEN_SNMP%"=="1"       set SYSGEN_TCPIP=1
    if "%SYSGEN_REDIR%"=="1"      set SYSGEN_TCPIP=1
    if "%SYSGEN_LDAP%"=="1"       set SYSGEN_TCPIP=1
    if "%SYSGEN_NETUTILS%"=="1"   set SYSGEN_TCPIP=1
    if "%SYSGEN_FIREWALL%"=="1"   set SYSGEN_IPHLPAPI=1
    if "%SYSGEN_IPHLPAPI%"=="1"   set SYSGEN_TCPIP=1

    REM Base TCP/IP support
    if "%SYSGEN_TCPIP%"=="1"    set CE_MODULES=%CE_MODULES% tcpstk
    if "%SYSGEN_TCPIP%"=="1"    set SYSGEN_NDIS=1
    if "%SYSGEN_TCPIP%"=="1"    set SYSGEN_WINSOCK=1
    if "%SYSGEN_TCPIP%"=="1"    set __SYSGEN_CXPORT=1
    if "%SYSGEN_TCPIP%"=="1"    set SYSGEN_FMTMSG=1
    if "%SYSGEN_TCPIP%"=="1"    set SYSGEN_FULL_CRT=1
    REM KW 6/4/2008 Remove dhcp
    REM if "%SYSGEN_TCPIP%"=="1"    set __SYSGEN_DHCP=1

    REM IPv6 support
    if "%SYSGEN_TCPIP6%"=="1"    set CE_MODULES=%CE_MODULES% tcpip6 ipv6hlp dhcpv6l
    if "%SYSGEN_TCPIP6%"=="1"    set SYSGEN_NDIS=1
    if "%SYSGEN_TCPIP6%"=="1"    set SYSGEN_WINSOCK=1
    if "%SYSGEN_TCPIP6%"=="1"    set __SYSGEN_CXPORT=1
    if "%SYSGEN_TCPIP6%"=="1"    set SYSGEN_FMTMSG=1
    if "%SYSGEN_TCPIP6%"=="1"    set SYSGEN_FULL_CRT=1

    REM IPv6 firewall
    if "%SYSGEN_FIREWALL%"=="1" set CE_MODULES=%CE_MODULES% fw6 fwapi
    if "%SYSGEN_FIREWALL%"=="1" set SYSGEN_WINSOCK=1


    REM Infrared (IrDA) support
    if "%SYSGEN_IRDA%"=="1"     set SYSGEN_NDIS=1
    if "%SYSGEN_IRDA%"=="1"     set SYSGEN_WINSOCK=1
    if "%SYSGEN_IRDA%"=="1"     set __SYSGEN_CXPORT=1
    if "%SYSGEN_IRDA%"=="1"     set CE_MODULES=%CE_MODULES% irdastk

    REM NDIS Logging Support.
    if "%SYSGEN_NETLOG%"=="1"   set SYSGEN_NDIS=1

    REM DnsApi support.  Brings in dnsapi.dll and component to support it in AFD.
    if not "%SYSGEN_DNSAPI%"=="1" goto noDnsapi
        set CE_MODULES=%CE_MODULES% dnsapi afd
        set AFD_COMPONENTS=%AFD_COMPONENTS% afddnslib
        set SYSGEN_DEVICE=1
        set SYSGEN_WINSOCK=1
        set __SYSGEN_CXPORT=1
        set SYSGEN_CORESTRA=1
        set __SYSGEN_NEED_LOCUSA=1
    :noDnsapi

    REM Winsock API support.  Winsock1.1 is now a stub that calls into WS2.
    if not "%SYSGEN_WINSOCK%"=="1"  goto noWinsock
        set CE_MODULES=%CE_MODULES% winsock afd
        set CE_MODULES=%CE_MODULES% ws2 ws2instl wspm nspm
        REM SSL winsock layered service provider
        if "%SYSGEN_AUTH_SCHANNEL%"=="1" set CE_MODULES=%CE_MODULES% ssllsp
        set SYSGEN_DEVICE=1
        set __SYSGEN_CXPORT=1
        set SYSGEN_CORESTRA=1
        set __SYSGEN_NEED_LOCUSA=1
    :noWinsock

    REM Built in (e.g. PCI only) NIC drivers. Include these always, will be filtered out by BSP_NIC_xxx settings
    if "%SYSGEN_ETHERNET%"=="1" set CE_MODULES=%CE_MODULES% rtl8139 e100bex smsc100fd dp83815
    REM NE2000 can be either installable (PCMCIA) or built in.  Include by default.
    if "%SYSGEN_ETHERNET%"=="1" set CE_MODULES=%CE_MODULES% ne2000

    if "%__SYSGEN_DHCP%"=="1" set CE_MODULES=%CE_MODULES% dhcp
    if "%__SYSGEN_DHCP%"=="1" set __SYSGEN_FILESYS=1
    if "%__SYSGEN_DHCP%"=="1" set SYSGEN_FMTMSG=1

    REM System tray icon for network status
    if "%SYSGEN_CONNMC%"=="1" if "%SYSGEN_ETHERNET%"=="1" set __SYSGEN_ETHMAN=1

    if "%__SYSGEN_ETHMAN%"=="1" set CE_MODULES=%CE_MODULES% ethman
    if "%__SYSGEN_ETHMAN%"=="1" set SYSGEN_NDISUIO=1

    if "%SYSGEN_SDBUS%"=="1" if "%SYSGEN_ETH_80211%"=="1" set CE_MODULES=%CE_MODULES% pegassdn

    REM if "%SYSGEN_ETH_80211%"=="1" set CE_MODULES=%CE_MODULES% wzcsvc wzcsapi pcx500 islp2nds wlclient wzctool ar6k_ndis_cf
    if "%SYSGEN_ETH_80211%"=="1" set CE_MODULES=%CE_MODULES% wzcsvc wzcsapi pcx500 islp2nds wlclient wzctool
    if "%SYSGEN_ETH_80211%"=="1" set __SYSGEN_EAPOL=1

    if "%__SYSGEN_EAPOL%"=="1" set CE_MODULES=%CE_MODULES% eapol
    if "%__SYSGEN_EAPOL%"=="1" set SYSGEN_EAP=1
    if "%SYSGEN_EAP%"=="1"   set CE_MODULES=%CE_MODULES% eap eapchap
    if "%SYSGEN_EAP%"=="1"   set __SYSGEN_CXPORT=1
    if "%SYSGEN_EAP%"=="1"   set SYSGEN_CORESTRA=1
    if "%SYSGEN_EAP%"=="1"   set SYSGEN_STDIOA=1
    if "%SYSGEN_EAP%"=="1"   set SYSGEN_STRSAFE=1
    if "%SYSGEN_EAP%"=="1" if "%SYSGEN_AUTH_SCHANNEL%"=="1" set CE_MODULES=%CE_MODULES% eaptls

    if "%SYSGEN_ETH_80211%"=="1" set SYSGEN_NDISUIO=1

    REM Nativewifi AP/STA.

    if "%SYSGEN_NWIFI_AP%"=="1"  set NWIFI_COMPONENTS=%NWIFI_COMPONENTS% ap  & set __SYSGEN_WIFI=1
    if "%SYSGEN_NWIFI_STA%"=="1" set NWIFI_COMPONENTS=%NWIFI_COMPONENTS% sta & set __SYSGEN_WIFI=1

    IF "%__SYSGEN_WIFI%"=="1" set CE_MODULES=%CE_MODULES% wlclient wlsvc oneex nwifi rtl8180wf wzcsapi
    


    REM PCMCIA NDIS miniport drivers
    REM KW 6/4/2008 Remove netmui
    REM if "%SYSGEN_ETHERNET%"=="1" set CE_MODULES=%CE_MODULES% netmui

    REM NDIS interface module
    if "%SYSGEN_NDISUIO%"=="1" set CE_MODULES=%CE_MODULES% ndisuio
    if "%SYSGEN_NDISUIO%"=="1" set SYSGEN_NDIS=1

    REM NDIS driver support
    if "%SYSGEN_NDIS%"=="1"     set SYSGEN_DEVICE=1
    if "%SYSGEN_NDIS%"=="1"     set SYSGEN_MSGQUEUE=1
    if "%SYSGEN_NDIS%"=="1"     set SYSGEN_CORESTRA=1
    if "%SYSGEN_NDIS%"=="1"     set SYSGEN_STDIOA=1
    if "%SYSGEN_NDIS%"=="1"     set SYSGEN_STRSAFE=1
    if "%SYSGEN_NDIS%"=="1"     set __SYSGEN_CXPORT=1
    if "%SYSGEN_NDIS%"=="1"     set CE_MODULES=%CE_MODULES% ndis

    REM Internet Connection Sharing
    if "%SYSGEN_GATEWAY%"=="1" set CE_MODULES=%CE_MODULES% ipnat
    if "%SYSGEN_GATEWAY%"=="1" set AFD_COMPONENTS=%AFD_COMPONENTS% afddnsproxy

    REM MAC Bridge
    if "%SYSGEN_BRIDGE%"=="1" set CE_MODULES=%CE_MODULES% mbridge
    if "%SYSGEN_BRIDGE2%"=="1" set CE_MODULES=%CE_MODULES% mbridge2

    if "%SYSGEN_AUTORAS%"=="1" set CE_MODULES=%CE_MODULES% autoras

    if not "%SYSGEN_PPP%" == "1" goto no_ppp
       REM PPP (Point-to-Point Protocol)
       set SYSGEN_MSGQUEUE=1

       set CE_MODULES=%CE_MODULES% ppp

       REM Optional features that are only interesting when PPP is present
       if "%SYSGEN_GATEWAY%"=="1" set CE_MODULES=%CE_MODULES% autodial
       if "%SYSGEN_IRDA%"=="1" set CE_MODULES=%CE_MODULES% ircomm
       set SYSGEN_MINGWES=1

       if     "%SYSGEN_PPP_SERVER%"=="1"     set PPP_COMPONENTS=%PPP_COMPONENTS% ppp2srv
       if not "%SYSGEN_PPP_SERVER%"=="1"     set PPP_COMPONENTS=%PPP_COMPONENTS% ppp2srvstub

    :no_ppp


    REM //Communications apps
    if "%SYSGEN_PEGTERM%"=="1" set CE_MODULES=%CE_MODULES% pegterm
    if "%SYSGEN_PEGTERM%"=="1" set __SYSGEN_TERMCTRL=1
    if "%SYSGEN_PEGTERM%"=="1" set SYSGEN_TAPI=1
    if "%SYSGEN_PEGTERM%"=="1" set __SYSGEN_FULLGWES=1

    REM Terminal Control needs a fixed point font.
    if "%__SYSGEN_TERMCTRL%"=="1" set CE_MODULES=%CE_MODULES% termctrl
    if "%__SYSGEN_TERMCTRL%"=="1" set __SYSGEN_FONTS_ANY_COUR=1

    REM TAPI dialing support.
    if "%SYSGEN_TAPI%" == "1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% tapilib
    if "%SYSGEN_TAPI%" == "1" set CE_MODULES=%CE_MODULES% tapi
    if "%SYSGEN_TAPI%" == "1" set __SYSGEN_FILESYS=1
    if "%SYSGEN_TAPI%" == "1" set SYSGEN_MINGWES=1
    if "%SYSGEN_TAPI%" == "1" set SYSGEN_DEVICE=1
    if "%SYSGEN_PPTP%"=="1" set CE_MODULES=%CE_MODULES% pptp
    if "%SYSGEN_L2TP%"=="1" set CE_MODULES=%CE_MODULES% l2tp

    if "%SYSGEN_PPPOE%"=="1" set CE_MODULES=%CE_MODULES% pppoe

    if "%SYSGEN_IPSEC%"=="1" set CE_MODULES=%CE_MODULES% ipsec ipsecsvc ipseccfg


    if "%SYSGEN_SNMP%"=="1" set CE_MODULES=%CE_MODULES% snmp snmp_mibii snmp_hostmib
    REM HostMIB requires toolhelp
    if "%SYSGEN_SNMP%"=="1" set SYSGEN_TOOLHELP=1
    if "%SYSGEN_SNMP%"=="1" set SYSGEN_STRSAFE=1
    if "%SYSGEN_SNMP%"=="1" set SYSGEN_STDIOA=1

    REM // Redirector
    if "%SYSGEN_REDIR%"=="1" set __SYSGEN_NETBIOS=1
    if "%SYSGEN_REDIR%"=="1" set SYSGEN_STRSAFE=1
    if "%SYSGEN_REDIR%"=="1" set SYSGEN_STDIOA=1

    if "%SYSGEN_REDIR%"=="1" set CE_MODULES=%CE_MODULES% redir
    if "%__SYSGEN_NETBIOS%"=="1" set CE_MODULES=%CE_MODULES% netbios

    REM // Event logging
    if "%__SYSGEN_EVENTLOG%"=="1" set CE_MODULES=%CE_MODULES% eventlog eventlogmsgs

    REM // Bluetooth
    if not "%__SYSGEN_BTH%"=="1" goto endbluetooth
        if "%SYSGEN_BTH_GATEWAY%"=="1" set CE_MODULES=%CE_MODULES% btgw btdun
        if "%SYSGEN_BTH_UTILS%"=="1" set CE_MODULES=%CE_MODULES% btloader
        if "%SYSGEN_BTH_MODEM%"=="1" set CE_MODULES=%CE_MODULES% btmodem
        REM if "%__SYSGEN_NETUI%"=="1" if "%SYSGEN_STANDARDSHELL%"=="1" set CE_MODULES=%CE_MODULES% btsvc
        if "%__SYSGEN_NETUI%"=="1" set CE_MODULES=%CE_MODULES% btsvc
        set __SYSGEN_NEED_LOCUSA=1
        set CE_MODULES=%CE_MODULES% btd btdrt
        set BTD_COMPONENTS=hci l2cap sdp rfcomm portemu sys
        set BTDRT_COMPONENTS=sdpuser

        set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% tbtcore

        if "%SYSGEN_WINSOCK%"=="1" set BTDRT_COMPONENTS=%BTDRT_COMPONENTS% bthns
        if "%SYSGEN_WINSOCK%"=="1" set BTD_COMPONENTS=%BTD_COMPONENTS% tdi
        if "%SYSGEN_BTH_PAN%"=="1" set BTD_COMPONENTS=%BTD_COMPONENTS% btpan
        REM if "%SYSGEN_BTH_AVDTP%"=="1" set BTD_COMPONENTS=%BTD_COMPONENTS% avdtp
        REM if "%SYSGEN_BTH_AVCTP%"=="1" set BTD_COMPONENTS=%BTD_COMPONENTS% avctp
        REM if "%SYSGEN_BTH_AVRCP%"=="1" set BTD_COMPONENTS=%BTD_COMPONENTS% avrcp

        if "%__SYSGEN_BTH_HID%"=="1" set CE_MODULES=%CE_MODULES% bthhid
        REM if "%SYSGEN_BTH_A2DP%"=="1" set CE_MODULES=%CE_MODULES% bta2dp sbc

        if not "%SYSGEN_BTH_AG%"=="1" goto endbtag
            set SYSGEN_AUDIO=1
            if "%REPLACE_SYSGEN_BTH_AG%"=="1" goto no_btag_components
                set CE_MODULES=%CE_MODULES% btagsvc
                set BTAGSVC_COMPONENTS=btagsvc_network btagsvc_phoneext btagsvc_bond
                goto endbtag
            :no_btag_components
            set REPLACE_MODULES=%REPLACE_MODULES% btagsvc
        :endbtag

        if not "%SYSGEN_BTH_AUDIO%"=="1" goto nobtaudio
            set SYSGEN_AUDIO=1
            set CE_MODULES=%CE_MODULES% btscosnd
        :nobtaudio

        REM Delivered PCMCIA driver samples do not notify the stack
        REM of the insertion events. They are best used with universal
        REM driver only.

        if "%SYSGEN_BTH_UART_ONLY%"=="1" goto :bth_uart_only
        if "%SYSGEN_BTH_USB_ONLY%"=="1" goto :bth_usb_only
        if "%SYSGEN_BTH_CSR_ONLY%"=="1" goto :bth_csr_only
        if "%SYSGEN_BTH_SDIO_ONLY%"=="1" goto :bth_sdio_only

        REM Include all of the BTH Drivers.
        set BTD_COMPONENTS=%BTD_COMPONENTS% univ
        set BTD_DRIVERS=bthuart bthamb bthsc bthcsr bthuniv wendyser wcestreambt sio950
        if "%SYSGEN_USB%"=="1" set BTD_DRIVERS=%BTD_DRIVERS% bthusb
        if "%SYSGEN_SDBUS%"=="1" set BTD_DRIVERS=%BTD_DRIVERS% bthsdio
        set CE_MODULES=%CE_MODULES% %BTD_DRIVERS%
        goto endbluetooth

    :bth_uart_only
        set BTD_COMPONENTS=%BTD_COMPONENTS% uart
        goto endbluetooth

    :bth_csr_only
        set BTD_COMPONENTS=%BTD_COMPONENTS% csr
        goto endbluetooth

    :bth_usb_only
        set BTD_COMPONENTS=%BTD_COMPONENTS% usb
        set BTD_DRIVERS=bthusb
        set CE_MODULES=%CE_MODULES% %BTD_DRIVERS%
        goto endbluetooth

    :bth_sdio_only
        set BTD_COMPONENTS=%BTD_COMPONENTS% sdio
        set BTD_DRIVERS=bthsdio
        set CE_MODULES=%CE_MODULES% %BTD_DRIVERS%
        goto endbluetooth

    :endbluetooth

    REM Network utilities
    if "%SYSGEN_NETUTILS%"=="1" set CE_MODULES=%CE_MODULES% ping ipconfig tracert route netstat ndisconfig
    if "%SYSGEN_NETUTILS%"=="1" if "%SYSGEN_TCPIP6%"=="1" set CE_MODULES=%CE_MODULES% ipv6 ipv6tun
    REM IPv6.exe requires stdio
    if "%SYSGEN_NETUTILS%"=="1" if "%SYSGEN_TCPIP6%"=="1" set SYSGEN_STDIOA=1

    if "%SYSGEN_IPHLPAPI%"=="1" set CE_MODULES=%CE_MODULES% iphlpapi

    if "%SYSGEN_1394_TOOLS%"=="1" set __SYSGEN_FULLGWES=1

    if "%SYSGEN_NETLOG%"=="1" set CE_MODULES=%CE_MODULES% netlog netlogctl


    REM // ACM Codecs/MSFilter - sample ACM codec w/ Echo,Volume effects
    if "%SYSGEN_ACM_MSFILTER%"=="1" set CE_MODULES=%CE_MODULES% msfilter
    if "%SYSGEN_ACM_MSFILTER%"=="1" set SYSGEN_AUDIO_ACM=1
    if "%SYSGEN_ACM_MSFILTER%"=="1" set SYSGEN_MINWMGR=1

    if not "%SYSGEN_KEYBDTEST%"=="1" goto noKeybdTest
        set CE_MODULES=%CE_MODULES% kbdtest
        set SYSGEN_MININPUT=1
        set SYSGEN_MINGDI=1
    :noKeybdTest

    if not "%SYSGEN_LARGEKB%"=="1" goto nolargekb
       set SYSGEN_SOFTKB=1
       set __SYSGEN_FULLGWES=1
       set CE_MODULES=%CE_MODULES% largekb
    :nolargekb

    REM // Multimon components
    if not "%SYSGEN_MULTIMON%"=="1" goto noMultiMon
        set CE_MODULES=%CE_MODULES% multimon
        set SYSGEN_MINGDI=1
    :noMultiMon


    REM // Pull in the SIP for these locales
    set __SYSGEN_CHINESE_SIMPLIFIED=
    set __SYSGEN_CHINESE_TRADITIONAL=


    REM // These are ignored if no GWES components are selected
    if "%LOCALE%"=="0804" set __SYSGEN_CHINESE_SIMPLIFIED=1
    if "%LOCALE%"=="1004" set __SYSGEN_CHINESE_SIMPLIFIED=1
    if "%LOCALE%"=="0404" set __SYSGEN_CHINESE_TRADITIONAL=1
    if "%LOCALE%"=="0C04" set __SYSGEN_CHINESE_TRADITIONAL=1
    if "%LOCALE%"=="1404" set __SYSGEN_CHINESE_TRADITIONAL=1

	REM // Pull in the SIP for Korean IMs
	if "%SYSGEN_MSIMK%" == "1" set SYSGEN_SOFTKB=1
	if "%SYSGEN_MBOXKOR%" == "1" set SYSGEN_SOFTKB=1
	
	REM // Pull in the SIP for Chinese IMs
	if "%SYSGEN_PHONIM%" == "1" set SYSGEN_SOFTKB=1
	if "%SYSGEN_CHAJEIIM%" == "1" set SYSGEN_SOFTKB=1
	if "%SYSGEN_SPIM%" == "1" set SYSGEN_SOFTKB=1
	if "%SYSGEN_SPIM_MSPY%" == "1" set SYSGEN_SOFTKB=1
	if "%SYSGEN_MBOXCHT%" == "1" set SYSGEN_SOFTKB=1
	
    REM // Pull in the SIP for Japanese IMs
    if "%SYSGEN_CACJPN%" == "1" set SYSGEN_SOFTKB=1
    if "%SYSGEN_MULTIBOX%" == "1" set SYSGEN_SOFTKB=1
    if "%SYSGEN_IM_ALLCHAR%"=="1"  set SYSGEN_SOFTKB=1
    if "%SYSGEN_IM_RADICAL%"=="1"  set SYSGEN_SOFTKB=1
    if "%SYSGEN_IM_STROKE%"=="1"  set SYSGEN_SOFTKB=1
    if "%SYSGEN_IM_KANA%"=="1"  set SYSGEN_SOFTKB=1
    if "%SYSGEN_IM_ROMA%"=="1"  set SYSGEN_SOFTKB=1

	REM // Korean & Chinese IMEs need coresip
	if "%SYSGEN_K_IME97%" == "1" set __SYSGEN_CORESIP=1
	if "%SYSGEN_PIME_SC%" == "1" set __SYSGEN_CORESIP=1
	if "%SYSGEN_PIME_TC%" == "1" set __SYSGEN_CORESIP=1
	if "%SYSGEN_MSPY3_SC%" == "1" set __SYSGEN_CORESIP=1
	
    REM // MSPY3 needs this module
    if "%SYSGEN_MSPY3_SC%" == "1" set SYSGEN_CPP_EH_AND_RTTI=1

    REM // If you select the SIP pull in the default IM and the SIP control panel applet
    if not "%SYSGEN_SOFTKB%"=="1" goto nosoftkb
        set CE_MODULES=%CE_MODULES% softkb
        set __SYSGEN_CORESIP=1
        set SYSGEN_IMM=1
        set __SYSGEN_FULLGWES=1
    :nosoftkb

    if not "%__SYSGEN_CORESIP%"=="1" goto no_coresip
        set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% coresip
    :no_coresip

    REM // Keyboard layouts
    if "%SYSGEN_KBD_ARABIC_101%"=="1"   set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_ARABIC_101%"=="1"   set CE_MODULES=%CE_MODULES% kbda1
    if "%SYSGEN_KBD_HEBREW%"=="1"       set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_HEBREW%"=="1"       set CE_MODULES=%CE_MODULES% kbdheb
    if "%SYSGEN_KBD_THAI_KEDMANEE%"=="1"        set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_THAI_KEDMANEE%"=="1"        set CE_MODULES=%CE_MODULES% kbdth0
    if "%SYSGEN_KBD_HINDI_TRADITIONAL%"=="1"    set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_HINDI_TRADITIONAL%"=="1"    set CE_MODULES=%CE_MODULES% kbdinhin
    if "%SYSGEN_KBD_MARATHI%"=="1"      set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_MARATHI%"=="1"      set CE_MODULES=%CE_MODULES% kbdinmar
    if "%SYSGEN_KBD_PUNJABI%"=="1"      set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_PUNJABI%"=="1"      set CE_MODULES=%CE_MODULES% kbdinpun
    if "%SYSGEN_KBD_TELUGU%"=="1"       set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_TELUGU%"=="1"       set CE_MODULES=%CE_MODULES% kbdintel
    if "%SYSGEN_KBD_GUJARATI%"=="1"     set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_GUJARATI%"=="1"     set CE_MODULES=%CE_MODULES% kbdinguj
    if "%SYSGEN_KBD_KANNADA%"=="1"      set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_KANNADA%"=="1"      set CE_MODULES=%CE_MODULES% kbdinkan
    if "%SYSGEN_KBD_TAMIL%"=="1"        set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_TAMIL%"=="1"        set CE_MODULES=%CE_MODULES% kbdintam
    if "%SYSGEN_KBD_US_DVORAK%"=="1"    set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_US_DVORAK%"=="1"    set CE_MODULES=%CE_MODULES% kbddv
    if "%SYSGEN_KBD_US%"=="1"           set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_US%"=="1"           set CE_MODULES=%CE_MODULES% kbdus
    if "%SYSGEN_KBD_JAPANESE%"=="1"     set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_JAPANESE%"=="1"     set CE_MODULES=%CE_MODULES% kbdjpn kbdjpn1
    if "%SYSGEN_KBD_KOREAN%"=="1"       set SYSGEN_MINWMGR=1
    if "%SYSGEN_KBD_KOREAN%"=="1"       set CE_MODULES=%CE_MODULES% kbdkor


    REM ==============================================================================================
    REM
    REM Optional components from GWES.  Certain components require basic window message handling
    REM
    REM ==============================================================================================

    REM Accessib requires full gwes support
    if "%SYSGEN_ACCESSIB%"=="1"  set __SYSGEN_FULLGWES=1
    if "%SYSGEN_PRINTING%"=="1"  set SYSGEN_MINGDI=1

    REM Full GWES corresponds to support in IABase
    if "%__SYSGEN_FULLGWES%"=="1"  set SYSGEN_MINGDI=1
    if "%__SYSGEN_FULLGWES%"=="1"  set SYSGEN_DISPLAY=1
    if "%__SYSGEN_FULLGWES%"=="1"  set SYSGEN_MININPUT=1
    if "%__SYSGEN_FULLGWES%"=="1"  set SYSGEN_MINWMGR=1
    if "%__SYSGEN_FULLGWES%"=="1"  set __SYSGEN_SHELL_APIS=1

    if "%__SYSGEN_SHELL_APIS%"=="1" set SYSGEN_MINWMGR=1
    if "%SYSGEN_GRADFILL%"=="1"  set SYSGEN_MINGDI=1
    if "%SYSGEN_GDI_ALPHABLEND%"=="1"  set SYSGEN_MINGDI=1
    if "%SYSGEN_GDI_FONTFIX%"=="1" set SYSGEN_MINGDI=1
    if "%SYSGEN_MENU_OVERLAP%"=="1" set SYSGEN_MINWMGR=1
    if "%SYSGEN_MINWMGR%"=="1"   set SYSGEN_MINGDI=1
    if "%SYSGEN_GDI_RASTERFONT%"=="1"  set SYSGEN_MINGDI=1
    if "%SYSGEN_AGFA_FONT%"=="1" set SYSGEN_MINGDI=1
    if "%SYSGEN_DISPLAY%"=="1"   set SYSGEN_MINGDI=1
    if "%SYSGEN_MINGDI%"=="1"    set SYSGEN_MININPUT=1
    if "%SYSGEN_MINWMGR%"=="1"   set SYSGEN_MININPUT=1
    if "%SYSGEN_MINGDI%"=="1"    set SYSGEN_MINGWES=1
    if "%SYSGEN_MINWMGR%"=="1"   set SYSGEN_MINGWES=1
    if "%SYSGEN_MININPUT%"=="1"  set SYSGEN_MINGWES=1

    if not "%SYSGEN_MINGWES%"=="1" goto noMinGWES
       REM // Minimal message only gwe + required client libraries for coredll
       set __SYSGEN_FILESYS=1
       set SYSGEN_DEVICE=1

       REM If a customer needs to replace a GWES component they should
       REM set REPLACE_SYSGEN_GWES
       if not "%REPLACE_SYSGEN_GWES%"=="1" set CE_MODULES=%CE_MODULES% gwes
       if "%REPLACE_SYSGEN_GWES%"=="1" set REPLACE_MODULES=%REPLACE_MODULES% gwes

       set GWE1_COMPONENTS=wmbase gweshare gwesmain immthunk msgque GSetWinLong CePtr
       set GWES_COMPONENTS=gwe1
       set GWE4_COMPONENTS=
       set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% rectapi wmgr_c

       set __SYSGEN_NEED_PMSTUBS=1
       if not "%SYSGEN_MININPUT%"=="1" goto noMinInput
          set CE_MODULES=%CE_MODULES% keybd pointer
          set GWE1_COMPONENTS=%GWE1_COMPONENTS% foregnd idle kbdui uibase
          set SYSGEN_FSPASSWORD=1
       :noMinInput

       if not "%SYSGEN_MINGDI%"=="1" goto noMinGDI
          set GWE1_COMPONENTS=%GWE1_COMPONENTS% msgbeep
          set GWE2_COMPONENTS=mgbase mgbitmap mgblt mgblt2 mgdc mgdibsec mgdraw mgrgn mgwinmgr
          REM // Can also have calibration UI now that we have a display
          set GWE2_COMPONENTS=%GWE2_COMPONENTS% tchui calibrui
          set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% mgdi_c
          REM Include TrueType engine
          if "%SYSGEN_GRADFILL%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% mggradfill
          if not "%SYSGEN_GRADFILL%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% mggradfillstub
          if "%SYSGEN_GDI_ALPHABLEND%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgalphablend
          if not "%SYSGEN_GDI_ALPHABLEND%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgalphablendstub
          if not "%SYSGEN_GDI_RASTERFONT%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgtt
          if "%SYSGEN_GDI_RASTERFONT%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgrast mgrast2
          if "%SYSGEN_GDI_RASTERFONT%"=="1" set FONTS_COMPONENTS=%FONTS_COMPONENTS% arial_raster
          if "%SYSGEN_AGFA_FONT%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% decompdrv

          if "%__SYSGEN_CHINESE_SIMPLIFIED%"=="1" set __SYSGEN_MGFE=1

          if "%__SYSGEN_CHINESE_TRADITIONAL%"=="1" set __SYSGEN_MGFE=1

          if "%LOCALE%" == "0411" set __SYSGEN_MGFE=1

          if "%LOCALE%" == "0412" set __SYSGEN_MGFE=1

          if "%__SYSGEN_MGFE%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgfe
          if "%SYSGEN_GDI_FONTFIX%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgfntfix

          REM Include DrawText API
          set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgdrwtxt

          REM Include a font as well
          if "%SYSGEN_GDI_RASTERFONT%"=="1" goto noTrueTypeFonts
            REM By default we include Tahoma font.  Here are all of the other possible fonts
            set FONTS_COMPONENTS=

            if not "%REPLACE_SYSGEN_DEFAULT_FONT%"=="1" set SYSGEN_FONTS_TAHOMA_1_07=1

            REM We require one version of Tahoma
            if not "%SYSGEN_FONTS_TAHOMA_1_08%"=="1" goto End_Tahoma_1_08
                set SYSGEN_FONTS_TAHOMA_1_07=
                set FONTS_COMPONENTS=%FONTS_COMPONENTS% tahoma_1_08
            :End_Tahoma_1_08

            if "%SYSGEN_FONTS_TAHOMA_1_07%"=="1" set FONTS_COMPONENTS=%FONTS_COMPONENTS% tahoma_1_07
            :End_Tahoma

            REM We require one version of Tahomabd
            if not "%SYSGEN_FONTS_TAHOMABD_1_08%"=="1" goto End_Tahomabd_1_08
                set SYSGEN_FONTS_TAHOMABD=
                set FONTS_COMPONENTS=%FONTS_COMPONENTS% tahomabd_1_08
            :End_Tahomabd_1_08

            if "%SYSGEN_FONTS_TAHOMABD%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% tahomabd
            :End_Tahomabd

            REM Some components (console, TermCtrl, really want a fixed point font.  If Cour hadn't been specified
            REM then include Cour_1_30 (the reduced version) automatically.
            if "%__SYSGEN_FONTS_ANY_COUR%"=="1"    set SYSGEN_FONTS_COUR_1_30=1

            REM We require one version of Cour
            if not "%SYSGEN_FONTS_COUR_1_08%"=="1" goto End_Cour_1_08
                set SYSGEN_FONTS_COUR_1_30=
                set FONTS_COMPONENTS=%FONTS_COMPONENTS% cour_1_08
            :End_Cour_1_08

            if "%SYSGEN_FONTS_COUR_1_30%"=="1"   set FONTS_COMPONENTS=%FONTS_COMPONENTS% cour_1_30
            :End_Cour


            REM Only include one of the following two
            if "%SYSGEN_FONTS_TIMES_1_30%"=="1"  set FONTS_COMPONENTS=%FONTS_COMPONENTS% times_1_30
            :End_Times

            REM We require one version of Arial
            if not "%SYSGEN_FONTS_ARIAL_1_08%"=="1" goto End_Arial_1_08
                set SYSGEN_FONTS_ARIAL_1_30=
                set FONTS_COMPONENTS=%FONTS_COMPONENTS% arial_1_08
            :End_Arial_1_08

            if "%SYSGEN_FONTS_ARIAL_1_30%"=="1"  set FONTS_COMPONENTS=%FONTS_COMPONENTS% arial_1_30
            :End_Arial

            REM We require one version of Arialbd
            if not "%SYSGEN_FONTS_ARIALBD_1_08%"=="1" goto End_Arialbd_1_08
                set SYSGEN_FONTS_ARIALBD=
                set FONTS_COMPONENTS=%FONTS_COMPONENTS% arialbd_1_08
            :End_Arialbd_1_08

            if "%SYSGEN_FONTS_ARIALBD%"=="1"     set FONTS_COMPONENTS=%FONTS_COMPONENTS% arialbd
            :End_Arialbd


            if "%SYSGEN_FONTS_SYMBOL%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% symbol
            if "%SYSGEN_FONTS_TIMESBD%"=="1"     set FONTS_COMPONENTS=%FONTS_COMPONENTS% timesbd

            if "%SYSGEN_FONTS_IMPACT%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% impact

            if "%SYSGEN_FONTS_TREBUCBD%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% trebucbd
            if "%SYSGEN_FONTS_COURBD%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% courbd
            if "%SYSGEN_FONTS_KINO%"=="1"        set FONTS_COMPONENTS=%FONTS_COMPONENTS% kino
            if "%SYSGEN_FONTS_TREBUCBI%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% trebucbi
            if "%SYSGEN_FONTS_COURBI%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% courbi
            if "%SYSGEN_FONTS_TREBUCIT%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% trebucit
            if "%SYSGEN_FONTS_ARIALBI%"=="1"     set FONTS_COMPONENTS=%FONTS_COMPONENTS% arialbi
            if "%SYSGEN_FONTS_COURI%"=="1"       set FONTS_COMPONENTS=%FONTS_COMPONENTS% couri
            if "%SYSGEN_FONTS_MSLOGO%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% mslogo
            if "%SYSGEN_FONTS_VERDANA%"=="1"     set FONTS_COMPONENTS=%FONTS_COMPONENTS% verdana
            if "%SYSGEN_FONTS_ARIALI%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% ariali
            if "%SYSGEN_FONTS_GEORGIA%"=="1"     set FONTS_COMPONENTS=%FONTS_COMPONENTS% georgia

            if "%SYSGEN_FONTS_MANGAL%"=="1"     set FONTS_COMPONENTS=%FONTS_COMPONENTS% mangal
            if "%SYSGEN_FONTS_LATHA%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% latha
            if "%SYSGEN_FONTS_GAUTAMI%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% gautami
            if "%SYSGEN_FONTS_RAAVI%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% raavi
            if "%SYSGEN_FONTS_SHRUTI%"=="1"     set FONTS_COMPONENTS=%FONTS_COMPONENTS% shruti
            if "%SYSGEN_FONTS_TUNGA%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% tunga

            REM Only include one of the following
            if "%SYSGEN_FONTS_MSMING%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% msming&& goto End_MSMING
            :End_MSMING

            if "%SYSGEN_FONTS_VERDANAB%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% verdanab
            if "%SYSGEN_FONTS_ARIALK%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% arialk
            if "%SYSGEN_FONTS_GEORGIAB%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% georgiab
            if "%SYSGEN_FONTS_SUNFON%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% sunfon
            if "%SYSGEN_FONTS_VERDANAI%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% verdanai
            if "%SYSGEN_FONTS_COMIC%"=="1"       set FONTS_COMPONENTS=%FONTS_COMPONENTS% comic
            if "%SYSGEN_FONTS_GEORGIAI%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% georgiai
            if "%SYSGEN_FONTS_TIMESBI%"=="1"     set FONTS_COMPONENTS=%FONTS_COMPONENTS% timesbi
            if "%SYSGEN_FONTS_VERDANAZ%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% verdanaz
            if "%SYSGEN_FONTS_COMICBD%"=="1"     set FONTS_COMPONENTS=%FONTS_COMPONENTS% comicbd
            if "%SYSGEN_FONTS_GEORGIAZ%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% georgiaz
            if "%SYSGEN_FONTS_TIMESI%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% timesi
            if "%SYSGEN_FONTS_WEBDINGS%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% webdings

            REM Only include one of the following
            if "%SYSGEN_FONTS_GL_CE%"=="1"       set FONTS_COMPONENTS=%FONTS_COMPONENTS% gl_ce&& goto End_GL_CE
            :End_GL_CE

            if "%SYSGEN_FONTS_TREBUC%"=="1"      set FONTS_COMPONENTS=%FONTS_COMPONENTS% trebuc
            if "%SYSGEN_FONTS_WINGDING%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% wingding


            REM Only select one of the following eight
            if not "%SYSGEN_FONTS_MSGOTHIC30%"=="1"		 goto End_MSGOTHIC30
	            set SYSGEN_FONTS_MSGOTHIC30_1_19=
	            set SYSGEN_FONTS_MSGOTHIC_1_50=
	            set SYSGEN_FONTS_MSGOTHIC_1_60=
	            set SYSGEN_FONTS_MSGOTHIC_1_70=
	            set SYSGEN_FONTS_MSGOTHIC_1_80=
	            set SYSGEN_FONTS_MSGOTHIC_1_90=
	            set SYSGEN_FONTS_MSGOTHIC=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% msgothic30
            	goto End_MSGothic
            :End_MSGOTHIC30

            if not "%SYSGEN_FONTS_MSGOTHIC30_1_19%"=="1" goto End_MSGOTHIC30_1_19
	            REM set SYSGEN_FONTS_MSGOTHIC30=
	            set SYSGEN_FONTS_MSGOTHIC_1_50=
	            set SYSGEN_FONTS_MSGOTHIC_1_60=
	            set SYSGEN_FONTS_MSGOTHIC_1_70=
	            set SYSGEN_FONTS_MSGOTHIC_1_80=
	            set SYSGEN_FONTS_MSGOTHIC_1_90=
	            set SYSGEN_FONTS_MSGOTHIC=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% msgothic30_1_19
            	goto End_MSGothic
            :End_MSGOTHIC30_1_19

            if not "%SYSGEN_FONTS_MSGOTHIC_1_50%"=="1"   goto End_MSGOTHIC_1_50
	            REM set SYSGEN_FONTS_MSGOTHIC30=
	            REM set SYSGEN_FONTS_MSGOTHIC30_1_19=
	            set SYSGEN_FONTS_MSGOTHIC_1_60=
	            set SYSGEN_FONTS_MSGOTHIC_1_70=
	            set SYSGEN_FONTS_MSGOTHIC_1_80=
	            set SYSGEN_FONTS_MSGOTHIC_1_90=
	            set SYSGEN_FONTS_MSGOTHIC=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% msgothic_1_50
            	goto End_MSGothic
            :End_MSGOTHIC_1_50

            if not "%SYSGEN_FONTS_MSGOTHIC_1_60%"=="1"   goto End_MSGOTHIC_1_60
	            REM set SYSGEN_FONTS_MSGOTHIC30=
	            REM set SYSGEN_FONTS_MSGOTHIC30_1_19=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_50=
	            set SYSGEN_FONTS_MSGOTHIC_1_70=
	            set SYSGEN_FONTS_MSGOTHIC_1_80=
	            set SYSGEN_FONTS_MSGOTHIC_1_90=
	            set SYSGEN_FONTS_MSGOTHIC=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% msgothic_1_60
            	goto End_MSGothic
            :End_MSGOTHIC_1_60

            if not "%SYSGEN_FONTS_MSGOTHIC_1_70%"=="1"   goto End_MSGOTHIC_1_70
	            REM set SYSGEN_FONTS_MSGOTHIC30=
	            REM set SYSGEN_FONTS_MSGOTHIC30_1_19=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_50=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_60=
	            set SYSGEN_FONTS_MSGOTHIC_1_80=
	            set SYSGEN_FONTS_MSGOTHIC_1_90=
	            set SYSGEN_FONTS_MSGOTHIC=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% msgothic_1_70
            	goto End_MSGothic
            :End_MSGOTHIC_1_70

            if not "%SYSGEN_FONTS_MSGOTHIC_1_80%"=="1"   goto End_MSGOTHIC_1_80
	            REM set SYSGEN_FONTS_MSGOTHIC30=
	            REM set SYSGEN_FONTS_MSGOTHIC30_1_19=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_50=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_60=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_70=
	            set SYSGEN_FONTS_MSGOTHIC_1_90=
	            set SYSGEN_FONTS_MSGOTHIC=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% msgothic_1_80
            	goto End_MSGothic
            :End_MSGOTHIC_1_80

            if not "%SYSGEN_FONTS_MSGOTHIC_1_90%"=="1"   goto End_MSGOTHIC_1_90
	            REM set SYSGEN_FONTS_MSGOTHIC30=
	            REM set SYSGEN_FONTS_MSGOTHIC30_1_19=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_50=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_60=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_70=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_80=
	            set SYSGEN_FONTS_MSGOTHIC=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% msgothic_1_90
            	goto End_MSGothic
            :End_MSGOTHIC_1_90

            REM By default if LOCALE == 0411 (Japan) we want to include the full MSGothic font.
            if "%REPLACE_SYSGEN_DEFAULT_FONT%"=="1" goto no_msgothic
            if "%LOCALE%"=="0411"                  set SYSGEN_FONTS_MSGOTHIC=1
            :no_msgothic

            if not "%SYSGEN_FONTS_MSGOTHIC%"=="1"        goto End_MSGothic
	            REM set SYSGEN_FONTS_MSGOTHIC30=
	            REM set SYSGEN_FONTS_MSGOTHIC30_1_19=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_50=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_60=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_70=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_80=
	            REM set SYSGEN_FONTS_MSGOTHIC_1_90=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% msgothic
            :End_MSGothic


            if "%SYSGEN_FONTS_MSMINCHO%"=="1"    set FONTS_COMPONENTS=%FONTS_COMPONENTS% msmincho


            REM Only select one of the following seven
            if not "%SYSGEN_FONTS_SIMSUN_2_20%"=="1"     goto End_SIMSUN_2_20
	            set SYSGEN_FONTS_SIMSUN_2_50=
	            set SYSGEN_FONTS_SIMSUN_2_60=
	            set SYSGEN_FONTS_SIMSUN_2_70=
	            set SYSGEN_FONTS_SIMSUN_2_80=
	            set SYSGEN_FONTS_SIMSUN_2_90=
	            set SYSGEN_FONTS_SIMSUN=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% simsun_2_20
            	goto End_SimSun
            :End_SIMSUN_2_20

            if not "%SYSGEN_FONTS_SIMSUN_2_50%"=="1"     goto End_SIMSUN_2_50
	            REM set SYSGEN_FONTS_SIMSUN_2_20=
	            set SYSGEN_FONTS_SIMSUN_2_60=
	            set SYSGEN_FONTS_SIMSUN_2_70=
	            set SYSGEN_FONTS_SIMSUN_2_80=
	            set SYSGEN_FONTS_SIMSUN_2_90=
	            set SYSGEN_FONTS_SIMSUN=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% simsun_2_50
            	goto End_SimSun
            :End_SIMSUN_2_50

            if not "%SYSGEN_FONTS_SIMSUN_2_60%"=="1"     goto End_SIMSUN_2_60
	            REM set SYSGEN_FONTS_SIMSUN_2_20=
	            REM set SYSGEN_FONTS_SIMSUN_2_50=
	            set SYSGEN_FONTS_SIMSUN_2_70=
	            set SYSGEN_FONTS_SIMSUN_2_80=
	            set SYSGEN_FONTS_SIMSUN_2_90=
	            set SYSGEN_FONTS_SIMSUN=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% simsun_2_60
            	goto End_SimSun
            :End_SIMSUN_2_60

            if not "%SYSGEN_FONTS_SIMSUN_2_70%"=="1"     goto End_SIMSUN_2_70
	            REM set SYSGEN_FONTS_SIMSUN_2_20=
	            REM set SYSGEN_FONTS_SIMSUN_2_50=
	            REM set SYSGEN_FONTS_SIMSUN_2_60=
	            set SYSGEN_FONTS_SIMSUN_2_80=
	            set SYSGEN_FONTS_SIMSUN_2_90=
	            set SYSGEN_FONTS_SIMSUN=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% simsun_2_70
            	goto End_SimSun
            :End_SIMSUN_2_70

            if not "%SYSGEN_FONTS_SIMSUN_2_80%"=="1"     goto End_SIMSUN_2_80
	            REM set SYSGEN_FONTS_SIMSUN_2_20=
	            REM set SYSGEN_FONTS_SIMSUN_2_50=
	            REM set SYSGEN_FONTS_SIMSUN_2_60=
	            REM set SYSGEN_FONTS_SIMSUN_2_70=
	            set SYSGEN_FONTS_SIMSUN_2_90=
	            set SYSGEN_FONTS_SIMSUN=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% simsun_2_80
            	goto End_SimSun
            :End_SIMSUN_2_80

            if not "%SYSGEN_FONTS_SIMSUN_2_90%"=="1"     goto End_SIMSUN_2_90
	            REM set SYSGEN_FONTS_SIMSUN_2_20=
	            REM set SYSGEN_FONTS_SIMSUN_2_50=
	            REM set SYSGEN_FONTS_SIMSUN_2_60=
	            REM set SYSGEN_FONTS_SIMSUN_2_70=
	            REM set SYSGEN_FONTS_SIMSUN_2_80=
	            set SYSGEN_FONTS_SIMSUN=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% simsun_2_90
            	goto End_SimSun
            :End_SIMSUN_2_90

            REM If simplified Chinese we want to include SimSun_2_60 by default
            if "%REPLACE_SYSGEN_DEFAULT_FONT%"=="1" goto no_simsun
            if "%__SYSGEN_CHINESE_SIMPLIFIED%"=="1" set SYSGEN_FONTS_SIMSUN=1
            :no_simsun

            if not "%SYSGEN_FONTS_SIMSUN%"=="1"          goto End_SimSun
	            REM set SYSGEN_FONTS_SIMSUN_2_20=
	            REM set SYSGEN_FONTS_SIMSUN_2_50=
	            REM set SYSGEN_FONTS_SIMSUN_2_60=
	            REM set SYSGEN_FONTS_SIMSUN_2_70=
	            REM set SYSGEN_FONTS_SIMSUN_2_80=
	            REM set SYSGEN_FONTS_SIMSUN_2_90=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% simsun
            :End_SimSun


            REM Only select one of the following four
            if not "%SYSGEN_FONTS_MINGLIU_2_70%"=="1"    goto End_MINGLIU_2_70
            	set SYSGEN_FONTS_MINGLIU_2_80=
            	set SYSGEN_FONTS_MINGLIU_2_90=
            	set SYSGEN_FONTS_MINGLIU=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% mingliu_2_70
            	goto End_Mingliu
            :End_MINGLIU_2_70

            if not "%SYSGEN_FONTS_MINGLIU_2_80%"=="1"    goto End_MINGLIU_2_80
            	REM set SYSGEN_FONTS_MINGLIU_2_70=
            	set SYSGEN_FONTS_MINGLIU_2_90=
            	set SYSGEN_FONTS_MINGLIU=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% mingliu_2_80
            	goto End_Mingliu
            :End_MINGLIU_2_80

            if not "%SYSGEN_FONTS_MINGLIU_2_90%"=="1"    goto End_MINGLIU_2_90
            	REM set SYSGEN_FONTS_MINGLIU_2_70=
            	REM set SYSGEN_FONTS_MINGLIU_2_80=
            	set SYSGEN_FONTS_MINGLIU=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% mingliu_2_90
            	goto End_Mingliu
            :End_MINGLIU_2_90

            REM If traditional Chinese we want to include Mingliu by default
            if "%REPLACE_SYSGEN_DEFAULT_FONT%"=="1" goto no_mingliu
            if "%__SYSGEN_CHINESE_TRADITIONAL%"=="1" set SYSGEN_FONTS_MINGLIU=1
            :no_mingliu
            if not "%SYSGEN_FONTS_MINGLIU%"=="1"         goto End_Mingliu
            	REM set SYSGEN_FONTS_MINGLIU_2_70=
            	REM set SYSGEN_FONTS_MINGLIU_2_80=
            	REM set SYSGEN_FONTS_MINGLIU_2_90=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% mingliu
            :End_Mingliu


            REM Only select one of the following four
            if not "%SYSGEN_FONTS_GULIM_1_30%"=="1"      goto End_GULIM_1_30
            	set SYSGEN_FONTS_GULIM_1_40=
            	set SYSGEN_FONTS_GULIM_1_50=
            	set SYSGEN_FONTS_GULIM_1_60=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% gulim_1_30
            	goto End_Gulim
            :End_GULIM_1_30

            if not "%SYSGEN_FONTS_GULIM_1_40%"=="1"      goto End_GULIM_1_40
            	REM set SYSGEN_FONTS_GULIM_1_30=
            	set SYSGEN_FONTS_GULIM_1_50=
            	set SYSGEN_FONTS_GULIM_1_60=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% gulim_1_40
            	goto End_Gulim
            :End_GULIM_1_40

            if not "%SYSGEN_FONTS_GULIM_1_50%"=="1"      goto End_GULIM_1_50
            	REM set SYSGEN_FONTS_GULIM_1_30=
            	REM set SYSGEN_FONTS_GULIM_1_40=
            	set SYSGEN_FONTS_GULIM_1_60=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% gulim_1_50
            	goto End_Gulim
            :End_GULIM_1_50

            REM By default if LOCALE == 0412 (KOREA) we want to include Gulim_1_60
            if "%REPLACE_SYSGEN_DEFAULT_FONT%"=="1" goto no_gulim
            if "%LOCALE%"=="0412"                  set SYSGEN_FONTS_GULIM_1_60=1
            :no_gulim

            if not "%SYSGEN_FONTS_GULIM_1_60%"=="1"      goto End_Gulim
            	REM set SYSGEN_FONTS_GULIM_1_30=
            	REM set SYSGEN_FONTS_GULIM_1_40=
            	REM set SYSGEN_FONTS_GULIM_1_50=
            	set FONTS_COMPONENTS=%FONTS_COMPONENTS% gulim_1_60
            :End_Gulim
          :noTrueTypeFonts

          REM KW 6/5/2008 Include the mgprint always
          REM if "%SYSGEN_PRINTING%"=="1" set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgprint
          set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgprint

          REM Include palette support
          set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgpal mgpalnat
          set GWES_COMPONENTS=%GWES_COMPONENTS% gwe2
          REM Add the display driver support as well
          set CE_MODULES=%CE_MODULES% display fonts
          set __SYSGEN_NEED_LOCUSA=1
       :noMinGDI

       if not "%SYSGEN_MINWMGR%"=="1" goto noMinWMgr
          if "%REPLACE_SYSGEN_STARTUI%"=="1" goto no_startui
              set __SYSGEN_GWES_STARTUI=1
          :no_startui

          if "%REPLACE_SYSGEN_OOMUI%"=="1" goto no_oomui
              set __SYSGEN_GWES_OOMUI=1
          :no_oomui

          set GWE3_COMPONENTS=accel btnctl caret cascade imectl clipbd cmbctl defwndproc dlgmgr dlgmnem edctl gcache gwectrl icon iconcmn imgctl lbctl loadbmp loadimg menu menuscrl
          set GWE4_COMPONENTS=nclient oom sbcmn scbctl startup stcctl winmgr
          set GWES_COMPONENTS=%GWES_COMPONENTS% gwe3

          if "%__SYSGEN_GWES_STARTUI%"=="1" set GWE4_COMPONENTS=%GWE4_COMPONENTS% startui
          if "%__SYSGEN_GWES_OOMUI%"=="1" set GWE4_COMPONENTS=%GWE4_COMPONENTS% oomui

          REM // SYSGEN_REPLACESKIN should not be set unless OEM has defined the replacement skinning components
          if not "%GWES_REPLACE%"=="" goto noGwesReplace
          if not "%GWES_REPLACE_COMPONENTS%"=="" goto noGwesReplace
          set SYSGEN_REPLACESKIN=
          :noGwesReplace

          REM //XP or 9X UI
          if "%SYSGEN_REPLACESKIN%"=="1" goto ReplaceSkin
            if "%SYSGEN_XPSKIN%"=="1" set GWE4_COMPONENTS=%GWE4_COMPONENTS% sbcmnviewxp nclientviewxp gcacheviewxp btnctlviewxp stcctlviewxp cmbctlviewxp lbctlviewxp
            if not "%SYSGEN_XPSKIN%"=="1" set GWE4_COMPONENTS=%GWE4_COMPONENTS% sbcmnview nclientview gcacheview btnctlview stcctlview cmbctlview lbctlview
          :ReplaceSkin

          if not "%__SYSGEN_EDIMEFETPC%" == "1" goto NoEdImeFeTpc
              set GWE4_COMPONENTS=%GWE4_COMPONENTS% edimefetpc
              goto YesEdImeFeTpc

          :NoEdImeFeTpc
          if "%LOCALE%" == "0411" set GWE4_COMPONENTS=%GWE4_COMPONENTS% edimefe
          if "%LOCALE%" == "0412" set GWE4_COMPONENTS=%GWE4_COMPONENTS% edimefe
          if "%__SYSGEN_CHINESE_SIMPLIFIED%"=="1" set GWE4_COMPONENTS=%GWE4_COMPONENTS% edimefe
          if "%__SYSGEN_CHINESE_TRADITIONAL%"=="1" set GWE4_COMPONENTS=%GWE4_COMPONENTS% edimefe

          :YesEdImeFeTpc

          if "%SYSGEN_MENU_OVERLAP%"=="1" set GWE3_COMPONENTS=%GWE3_COMPONENTS% mOverlap
          if not "%SYSGEN_MENU_OVERLAP%"=="1" set GWE3_COMPONENTS=%GWE3_COMPONENTS% mNoOver

          set __SYSGEN_TOUCH_CURSOR=%SYSGEN_TOUCH%
          set __SYSGEN_MOUSE_CURSOR=%SYSGEN_CURSOR%

          REM __SYSGEN_MOUSE_AND_TOUCH_CURSOR sets both cursors
          if not "%__SYSGEN_MOUSE_AND_TOUCH_CURSOR%"=="1" goto NotDualCursor
            set __SYSGEN_MOUSE_CURSOR=1
            set __SYSGEN_TOUCH_CURSOR=1
          :NotDualCursor

          REM bsp settings override other cursor settings
          if "%BSP_NOCURSOR%"=="1" set __SYSGEN_MOUSE_CURSOR=
          if "%BSP_NOTOUCH%"=="1" set __SYSGEN_TOUCH_CURSOR=

          REM If still no mouse or touch, select mouse
          if "%__SYSGEN_TOUCH_CURSOR%"=="1" goto GotCursor
            set __SYSGEN_MOUSE_CURSOR=1
          :GotCursor

          REM Figure out which cursor routing component to use.
          if "%__SYSGEN_MOUSE_CURSOR%"=="1" goto MouseCursor
          if "%__SYSGEN_TOUCH_CURSOR%"=="1" set GWE3_COMPONENTS=%GWE3_COMPONENTS% TouchCursorOnly
          goto DoneCursorRouting

          :MouseCursor
          if "%__SYSGEN_TOUCH_CURSOR%"=="1"     set GWE3_COMPONENTS=%GWE3_COMPONENTS% MouseAndTouchCursor
          if not "%__SYSGEN_TOUCH_CURSOR%"=="1" set GWE3_COMPONENTS=%GWE3_COMPONENTS% MouseCursorOnly
          goto DoneCursorRouting

          :DoneCursorRouting
          REM KW 4/9/2009 Include iconcurs if Platform Manager is included (TestApp)
          REM if "%__SYSGEN_MOUSE_CURSOR%"=="1" set GWE3_COMPONENTS=%GWE3_COMPONENTS% iconcurs mcursor mcursor8
          if "%SYSGEN_PLATMAN%"=="1" set GWE3_COMPONENTS=%GWE3_COMPONENTS% iconcurs
          if "%__SYSGEN_MOUSE_CURSOR%"=="1" set GWE3_COMPONENTS=%GWE3_COMPONENTS% mcursor mcursor8
          if "%__SYSGEN_TOUCH_CURSOR%"=="1" set GWE3_COMPONENTS=%GWE3_COMPONENTS% cursor cursor8

          if "%__SYSGEN_GWE_CONTROLS%"=="OS"  set GWE4_COMPONENTS=%GWE4_COMPONENTS% msgbox msgbox_hpc dlgmgr_hpc menu_hpc EditControlOs
          if "%__SYSGEN_GWE_CONTROLS%"=="PPC" set GWE4_COMPONENTS=%GWE4_COMPONENTS% msgbox msgbox_ppc lbctl_ppc dlgmgr_ppc menu_ppc EditControlPpc
          if "%__SYSGEN_GWE_CONTROLS%"=="TPC" set GWE4_COMPONENTS=%GWE4_COMPONENTS% msgbox msgbox_tpc btnctl_tpc lbctl_tpc dlgmgr_tpc menu_tpc EditControlTpc

          if "%__SYSGEN_GWE_CONTROLS%"=="OS" goto doneOsGwesControls
          if "%__SYSGEN_MOUSE_CURSOR%"=="1" goto doneOsGwesControls
              set SYSGEN_MENU_TAP_UI=1
          :doneOsGwesControls

          if "%SYSGEN_MENU_TAP_UI%"=="1" goto tap_ui_menu
              set GWE3_COMPONENTS=%GWE3_COMPONENTS% mNoTapUI
              goto done_tap_ui
          :tap_ui_menu
              set GWE3_COMPONENTS=%GWE3_COMPONENTS% mTapUI
          :done_tap_ui


          set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% accel_c
          set __SYSGEN_AUDIO_API=1
          set __SYSGEN_GWE_TIMER=1
       :noMinWMgr
       if not "%SYSGEN_MINWMGR%"=="1" set __SYSGEN_MSGDLGBOXCUSTOMIZE=1

       if not "%__SYSGEN_GWE_TIMER%"=="1" goto noGweTimer
          set GWE4_COMPONENTS=%GWE4_COMPONENTS% timer
          set GWES_COMPONENTS=%GWES_COMPONENTS% gwe4
       :noGweTimer

       if not "%__SYSGEN_FULLGWES%"=="1" goto noFullGwes
          REM //Accessibility
          if "%SYSGEN_ACCESSIB%"=="1" set GWE1_COMPONENTS=%GWE1_COMPONENTS% accessib
          if "%SYSGEN_PIXELDOUBLE%"=="1" set GWE1_COMPONENTS=%GWE1_COMPONENTS% pixeldouble

          set GWE4_COMPONENTS=%GWE4_COMPONENTS% column atom drawmbar hotkey syscolor mgdx mgalias journal
          set SYSGEN_FSDBASE=1

          set __SYSGEN_GWE_MGTCI=1

          if not "%__SYSGEN_GWES_D3DHOOK%"=="1" goto noD3D
              REM // Gwes.exe Direct3D Mobile Support
              set GWE4_COMPONENTS=%GWE4_COMPONENTS% d3dmhook
              REM // Direct3D Mobile drivers.
              set CE_MODULES=%CE_MODULES% d3dmdrivers
          :noD3D
       :noFullGwes

    :noMinGWES

    if "%__SYSGEN_GWE_MGTCI%"=="1" set GWE4_COMPONENTS=%GWE4_COMPONENTS% mgtci

    REM If no GWES components then add the stubs
    if "%GWES_COMPONENTS%"=="" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% coregwestub
    if "%GWES_COMPONENTS%"=="" set __SYSGEN_MSGDLGBOXCUSTOMIZE=1

    if "%__SYSGEN_MSGDLGBOXCUSTOMIZE%"=="1" set COREDLL_MESSAGEDIALOGBOXCUSTOMIZE_COMPONENT=messagedialogboxcustomize
    if not "%__SYSGEN_MSGDLGBOXCUSTOMIZE%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% messagedialogboxthunk

    if not "%__SYSGEN_SHELL_APIS%"=="1" goto noShellAPIS
        REM Shell API components: SHMISC, SHEXEC, SHORTCUT, FILEOPEN
        set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% shcore shortcut shexec shmisc fileopen fileinfo shellapis
    :noShellAPIS

    if "%__SYSGEN_DSA%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% dsa

    if "%__SYSGEN_COREDLL_TIMEZONEINFO%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% timezones


    REM ==============================================================================================
    REM
    REM Optional audio components
    REM
    REM ==============================================================================================

    REM // WaveAPI - Waveform Audio, Hardware Mixer and Audio Compression Manager
    REM // ACM Codecs/GSM610 - handy for wireless
    if not "%SYSGEN_ACM_GSM610%"=="1" goto nogsm610
        set CE_MODULES=%CE_MODULES% gsm610
        set SYSGEN_AUDIO_ACM=1
    :nogsm610

    REM // ACM (Audio Compression Manager)
    if not "%SYSGEN_AUDIO_ACM%"=="1" goto noacm
        set WAVEAPI_COMPONENTS=%WAVEAPI_COMPONENTS% wapiacm
        set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% mmacm
        if "%__SYSGEN_FULLGWES%"=="1" set WAVEAPI_COMPONENTS=%WAVEAPI_COMPONENTS% wapiacmui
        if "%__SYSGEN_FULLGWES%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% mmacmui
        set SYSGEN_AUDIO=1
    :noacm

    if not "%SYSGEN_AUDIO%"=="1" goto noaudio
        set CE_MODULES=%CE_MODULES% waveapi audiodrv
        set CE_MODULES=%CE_MODULES% wavesamples
        if "%SYSGEN_MINWMGR%"=="1" set GWE1_COMPONENTS=%GWE1_COMPONENTS% audio
        set WAVEAPI_COMPONENTS=%WAVEAPI_COMPONENTS% wapiwave wapimap
        REM Audio stack needs DeviceIoControl, MsgQueues & Registry
        set __SYSGEN_FILESYS=1
        set SYSGEN_MSGQUEUE=1
        REM Also used GetDeviceKey - serdev.
        set SYSGEN_DEVICE=1
        set SYSGEN_SERDEV=1
        set __SYSGEN_NEED_LOCUSA=1
        set __SYSGEN_AUDIO_API=1
    :noaudio

    if "%__SYSGEN_AUDIO_API%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% mmwave mmsnd mmmix

    REM ==============================================================================================
    REM
    REM Optional Driver Components
    REM
    REM ==============================================================================================

    if "%SYSGEN_FSDMGR%"=="1"    set SYSGEN_STOREMGR=1

    REM // ++ Componentized ATA/ATAPI driver _________________________________________________________

    REM // atapi_common_lib is added to ATAPI_COMPONENTS implicitly

    if "%SYSGEN_ATAPI%"=="1"               set SYSGEN_DEVICE=1
    if "%SYSGEN_ATAPI%"=="1"               set SYSGEN_FATFS=1
    if "%SYSGEN_ATAPI%"=="1"               set SYSGEN_UDFS=1
    if "%SYSGEN_ATAPI%"=="1"               set SYSGEN_ATAPI_PCMCIA=1
    if "%SYSGEN_ATAPI%"=="1"               set SYSGEN_ATAPI_PCIO=1
    if "%SYSGEN_ATAPI%"=="1"               set SYSGEN_ATAPI_PCIO_CD=1
    if "%SYSGEN_ATAPI%"=="1"               set SYSGEN_ATAPI_PCIP_PDC20262=1

    if "%SYSGEN_ATAPI_PCIP_PDC20262%"=="1" set SYSGEN_DEVICE=1
    if "%SYSGEN_ATAPI_PCIP_PDC20262%"=="1" set SYSGEN_FATFS=1
    if "%SYSGEN_ATAPI_PCIP_PDC20262%"=="1" set SYSGEN_UDFS=1
    if "%SYSGEN_ATAPI_PCIP_PDC20262%"=="1" set SYSGEN_ATAPI_PCIO_CD=1
    if "%SYSGEN_ATAPI_PCIP_PDC20262%"=="1" set SYSGEN_ATAPI_PCIO=1

    if "%SYSGEN_ATAPI_PCIO_CD%"=="1"       set SYSGEN_DEVICE=1
    if "%SYSGEN_ATAPI_PCIO_CD%"=="1"       set SYSGEN_FATFS=1
    if "%SYSGEN_ATAPI_PCIO_CD%"=="1"       set SYSGEN_UDFS=1
    if "%SYSGEN_ATAPI_PCIO_CD%"=="1"       set SYSGEN_ATAPI_PCIO=1

    if "%SYSGEN_ATAPI_PCIO%"=="1"          set SYSGEN_DEVICE=1
    if "%SYSGEN_ATAPI_PCIO%"=="1"          set SYSGEN_FATFS=1

    if "%SYSGEN_ATAPI_PCMCIA%"=="1"        set SYSGEN_DEVICE=1
    if "%SYSGEN_ATAPI_PCMCIA%"=="1"        set SYSGEN_FATFS=1
    if "%SYSGEN_ATAPI_PCMCIA%"=="1"        set BSP_PCCARDATADISK=1

    REM // build ATAPI component set

    if "%SYSGEN_ATAPI_PCIO%"=="1"          set ATAPI_COMPONENTS=%ATAPI_COMPONENTS% atapi_pcio
    if "%SYSGEN_ATAPI_PCIO_CD%"=="1"       set ATAPI_COMPONENTS=%ATAPI_COMPONENTS% atapi_pcio_cd
    if "%SYSGEN_ATAPI_PCIP_PDC20262%"=="1" set ATAPI_COMPONENTS=%ATAPI_COMPONENTS% atapi_pcip_pdc20262
    if "%SYSGEN_ATAPI_PCMCIA%"=="1"        set ATAPI_COMPONENTS=%ATAPI_COMPONENTS% atapi_pcmcia

    REM // -- Componentized ATA/ATAPI driver ________________________________________________________

    if "%SYSGEN_ATADISK%"=="1"   set SYSGEN_DEVICE=1
    if "%SYSGEN_ATADISK%"=="1"   set SYSGEN_FATFS=1

    if "%SYSGEN_RAMDISK%"=="1"   set SYSGEN_STOREMGR=1

    if "%SYSGEN_SMARTCARD%"=="1" set SYSGEN_DEVICE=1
    if "%SYSGEN_USB%"=="1"       set SYSGEN_DEVICE=1
    if "%SYSGEN_PARALLEL%"=="1"  set SYSGEN_DEVICE=1

    REM // IEEE-1394
    if "%SYSGEN_1394_SBP2_SCSI%"=="1" set SYSGEN_1394_SBP2=1
    if "%SYSGEN_1394_SBP2%"=="1" set SYSGEN_1394=1
    if "%SYSGEN_1394_AVC_VCR_VIRTUAL%"=="1" set SYSGEN_1394_AVC_VIRTUAL=1
    if "%SYSGEN_1394_AVC_VIRTUAL%"=="1" set SYSGEN_1394_AVC_STREAMING=1
    if "%SYSGEN_1394_AVC_VCR%"=="1" set SYSGEN_1394_AVC_STREAMING=1
    if "%SYSGEN_1394_AVC_STREAMING%"=="1" set SYSGEN_1394_AVC=1
    if "%SYSGEN_1394_AVC%"=="1" set SYSGEN_1394=1
    if "%SYSGEN_1394_DCAM_VIRTUAL%"=="1" set SYSGEN_1394=1
    if "%SYSGEN_1394_DCAM%"=="1" set SYSGEN_1394=1
    if "%SYSGEN_1394_DIAG%"=="1" set SYSGEN_1394=1
    if "%SYSGEN_1394_DIAG_VIRTUAL%"=="1" set SYSGEN_1394=1
    if "%SYSGEN_1394_TOOLS%"=="1" set SYSGEN_1394=1
    if "%SYSGEN_1394%"=="1" set SYSGEN_DEVICE=1
    if "%SYSGEN_1394%"=="1" set SYSGEN_CORESTRA=1
    if "%SYSGEN_1394%"=="1" set __SYSGEN_CXPORT=1

    REM Helper library for TCP/IP, winsock, and 1394
    if "%__SYSGEN_CXPORT%"=="1"   set CE_MODULES=%CE_MODULES% cxport
    if "%__SYSGEN_CXPORT%"=="1"   set __SYSGEN_FILESYS=1
    if "%__SYSGEN_CXPORT%"=="1"   set SYSGEN_STDIOA=1

    REM // battery driver
    if not "%SYSGEN_BATTERY%"=="1" goto no_battery
       set SYSGEN_DEVICE=1
       set __SYSGEN_NEED_PMSTUBS=1
       set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% battery
       set CE_MODULES=%CE_MODULES% battdrvr
    :no_battery

    REM // notification LED driver
    if not "%SYSGEN_NLED%"=="1" goto no_nled
       set SYSGEN_DEVICE=1
       set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% nled
       set CE_MODULES=%CE_MODULES% nleddrvr
    :no_nled

    REM Null Camera driver
    if "%SYSGEN_CAMERA_NULL%"=="1" set SYSGEN_DEVICE=1
    if "%SYSGEN_CAMERA_NULL%"=="1" set CE_MODULES=%CE_MODULES% nullcam


    REM Bring in the device manager if power management is selected
    if "%SYSGEN_PM%"=="1" set SYSGEN_DEVICE=1
    if not "%SYSGEN_PM%"=="1" if "%__SYSGEN_NEED_PMSTUBS%"=="1" set SYSGEN_PMSTUBS=1

    REM If SYSGEN_PM then you can't have PMSTUBS
    if "%SYSGEN_PM%"=="1" set SYSGEN_PMSTUBS=

    if "%SYSGEN_PMSTUBS%"=="1" set SYSGEN_DEVICE=1

    if not "%SYSGEN_DEVICE%"=="1" goto no_device
       REM Compression is not technically needed at this point but if your image
       REM includes device.exe then it starts to be large enough that compression is useful
       set SYSGEN_NKCOMPR=1

       set SYSGEN_FULL_CRT=1
       set SYSGEN_FSADVERTISE=1
       set CE_MODULES=%CE_MODULES% device regenum busenum
       set DEVICE_COMPONENTS=%DEVICE_COMPONENTS% devcore iorm

       REM Select power manager components
       if not "%SYSGEN_PM%"=="1" goto not_full_PM
          set CE_MODULES=%CE_MODULES% pm
          if "%SYSGEN_PM_PDA%"=="1" set PM_COMPONENTS=%PM_COMPONENTS% pm_pda_pdd
          if not "%SYSGEN_PM_PDA%"=="1" set PM_COMPONENTS=%PM_COMPONENTS% pm_default_pdd
          set PM_COMPONENTS=%PM_COMPONENTS% pm_mdd pm_pdd_common
          set DEVICE_COMPONENTS=%DEVICE_COMPONENTS% pmif
          goto pm_done
       :not_full_PM
       if not "%SYSGEN_PMSTUBS%"=="1" goto no_PM
          set CE_MODULES=%CE_MODULES% pm
          set PM_COMPONENTS=%PM_COMPONENTS% pmstubs pm_pdd_common
          set DEVICE_COMPONENTS=%DEVICE_COMPONENTS% pmif
          goto pm_done
       :no_PM
          set DEVICE_COMPONENTS=%DEVICE_COMPONENTS% nopmif
          goto pm_done
       :pm_done

       set SYSGEN_SERDEV=1
       set SYSGEN_STDIO=1
       set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% devload

       REM // Driver DLLs compiled for all platforms.  Some of these are aliases for groups
       REM // of drivers that are linked at SYSGEN time -- see
       REM // public\common\oak\cesysgen\makefile for a list.
       set CE_MODULES=%CE_MODULES% ceddk giisr mmtimer pci
       set CE_MODULES=%CE_MODULES% pcmconv serial pccard

       REM // Driver DLLs that need some GWEs support
       if "%SYSGEN_MININPUT%"=="1" set CE_MODULES=%CE_MODULES% 8042keyboard nopkeyboard

       REM // Chip Support Package drivers.  The "csp" alias includes a number of sample drivers
       REM // that are linked at SYSGEN time -- see public\common\oak\cesysgen\makefile for a list.
       set CE_MODULES=%CE_MODULES% csp

       if "%SYSGEN_MININPUT%"=="1"     set CE_MODULES=%CE_MODULES% csp_mininput

       if "%SYSGEN_ATADISK%"=="1"      set CE_MODULES=%CE_MODULES% atadisk

       REM //=== FSD ===
       if not "%ATAPI_COMPONENTS%"=="" set CE_MODULES=%CE_MODULES% atapi
       if "%SYSGEN_RAMDISK%"=="1"  set CE_MODULES=%CE_MODULES% ramdisk

       REM //=== MSFLASH ===
       if not "%SYSGEN_MSFLASH%"=="1" goto :NotAllMSFlash
           set SYSGEN_MSFLASH_STRATAD=1
           set SYSGEN_MSFLASH_SDNPCID=1
           set SYSGEN_MSFLASH_FASLD=1
       :NotAllMSFlash
       REM // Add msflash drivers for individual flash technologies
       if "%SYSGEN_MSFLASH_STRATAD%"=="1" set CE_MODULES=%CE_MODULES% stratad
       if "%SYSGEN_MSFLASH_SDNPCID%"=="1" set CE_MODULES=%CE_MODULES% sdnpcid
       if "%SYSGEN_MSFLASH_FASLD%"=="1" set CE_MODULES=%CE_MODULES% fasld enumfaslpci
       if "%SYSGEN_MSFLASH_RAMFMD%"=="1" set CE_MODULES=%CE_MODULES% ramfmd

       REM // SD modules
       if "%SYSGEN_SDHC_STANDARD%"=="1" set CE_MODULES=%CE_MODULES% sdhc ellencfg
       REM  if "%SYSGEN_SDBUS%"=="1"         set CE_MODULES=%CE_MODULES% sdbus sdbus2
       if "%SYSGEN_SDBUS%"=="1"         set CE_MODULES=%CE_MODULES% sdbus

       REM // SD client modules
       if "%SYSGEN_SD_MEMORY%"=="1"     set CE_MODULES=%CE_MODULES% sdmemory

       REM // USB modules
       if "%SYSGEN_USB%"=="1"           set CE_MODULES=%CE_MODULES% usbhost usbd
       if "%SYSGEN_USB_HID%"=="1"       set CE_MODULES=%CE_MODULES% usbhid
       if "%SYSGEN_USB_PRINTER%"=="1"   set CE_MODULES=%CE_MODULES% usbprn
       if "%SYSGEN_USB_STORAGE%"=="1"   set CE_MODULES=%CE_MODULES% usbmsc usbdisk6
       if "%SYSGEN_ETH_USB_HOST%"=="1"  set CE_MODULES=%CE_MODULES% rndismp usb8023
       if "%SYSGEN_USB_SMARTCARD%"=="1" set CE_MODULES=%CE_MODULES% stcusb

       REM // HID modules
       if "%__SYSGEN_HID_PARSER%"=="1"     set CE_MODULES=%CE_MODULES% hidparse
       if "%__SYSGEN_HID_KEYBOARD%"=="1"   set CE_MODULES=%CE_MODULES% kbdhid conshid
       if "%__SYSGEN_HID_MOUSE%"=="1" if "%SYSGEN_TRANSCRIBER_MOUSE%"=="1" set CE_MODULES=%CE_MODULES% MouHidTrns
       if "%__SYSGEN_HID_MOUSE%"=="1" if "%SYSGEN_TRANSCRIBER_MOUSE%"==""  set CE_MODULES=%CE_MODULES% MouHid
       REM -- TBD make netui load FormatMessage dynamically
       if "%SYSGEN_USB%"=="1" set SYSGEN_FMTMSG=1

       REM // USB Function dependencies
       if "%SYSGEN_USBFN%"=="1"            set CE_MODULES=%CE_MODULES% usbfn
       if "%SYSGEN_USBFN_NET2280%"=="1"    set CE_MODULES=%CE_MODULES% net2280
       if "%SYSGEN_USBFN_STORAGE%"=="1"    set CE_MODULES=%CE_MODULES% usbmsfn
       if "%SYSGEN_USBFN_ETHERNET%"=="1"   set CE_MODULES=%CE_MODULES% rndisfn
       if "%SYSGEN_USBFN_SERIAL%"=="1"     set CE_MODULES=%CE_MODULES% serialusbfn

       REM // IEEE-1394 Components
       if "%SYSGEN_1394_DIAG%"=="1" set CE_MODULES=%CE_MODULES% 1394diag
       if "%SYSGEN_1394_DIAG_VIRTUAL%"=="1" set CE_MODULES=%CE_MODULES% 1394vdev
       if "%SYSGEN_1394_DCAM%"=="1" set CE_MODULES=%CE_MODULES% 1394dcam
       if "%SYSGEN_1394_DCAM_VIRTUAL%"=="1" set CE_MODULES=%CE_MODULES% virtdcam
       if "%SYSGEN_1394_AVC_VCR_VIRTUAL%"=="1" set CE_MODULES=%CE_MODULES% avc_vvcr
       if "%SYSGEN_1394_AVC_VIRTUAL%"=="1" set CE_MODULES=%CE_MODULES% avc_unit
       if "%SYSGEN_1394_AVC_VCR%"=="1" set CE_MODULES=%CE_MODULES% avc_vcr
       if "%SYSGEN_1394_AVC_STREAMING%"=="1" set CE_MODULES=%CE_MODULES% avc_stream
       if "%SYSGEN_1394_AVC%"=="1" set CE_MODULES=%CE_MODULES% avc 61883
       if "%SYSGEN_1394_SBP2_SCSI%"=="1" set CE_MODULES=%CE_MODULES% scsiblk
       if "%SYSGEN_1394_SBP2%"=="1" set CE_MODULES=%CE_MODULES% sbp2
       if "%SYSGEN_1394_TOOLS%"=="1" set CE_MODULES=%CE_MODULES% topomap
       if "%SYSGEN_1394%"=="1" set CE_MODULES=%CE_MODULES% setupapi wdmutil wdmlib cewdmmgr 1394bus 1394ohcd


    :no_device

    REM // FatFS support
    if "%SYSGEN_FATFS%"=="1"  set CE_MODULES=%CE_MODULES% fatfsd
    if "%SYSGEN_FATFS%"=="1"  set __SYSGEN_FAT_SUPPORT=1
    if "%SYSGEN_TFAT%"=="1"   set CE_MODULES=%CE_MODULES% tfat
    if "%SYSGEN_TFAT%"=="1"  set __SYSGEN_FAT_SUPPORT=1

    if not "%__SYSGEN_FAT_SUPPORT%"=="1" goto SkipFatSupport
        set SYSGEN_STOREMGR=1
        set SYSGEN_MSPART=1
        set __SYSGEN_NEED_LOCUSA=1
        set CE_MODULES=%CE_MODULES% fatutil diskcache
        if not "%__SYSGEN_FULLGWES%"=="1" set SYSGEN_FATUTIL_NOUI=1
        if "%SYSGEN_FATUTIL_NOUI%"=="1" set FATUTIL_COMPONENTS=fatutil_noui
        if not "%SYSGEN_FATUTIL_NOUI%"=="1" set FATUTIL_COMPONENTS=fatutil_ui
        set FATUTIL_COMPONENTS=%FATUTIL_COMPONENTS% fatutil_main
        if "%__SYSGEN_FULLGWES%"=="1" set SYSGEN_FMTMSG=1
    :SkipFatSupport

    if "%SYSGEN_UDFS%"=="1"      set SYSGEN_STOREMGR=1
    if "%SYSGEN_UDFS%"=="1"   set CE_MODULES=%CE_MODULES% udfs

    if "%SYSGEN_BINFS%"=="1"     set SYSGEN_STOREMGR=1
    if "%SYSGEN_BINFS%"=="1"  set CE_MODULES=%CE_MODULES% binfs

    REM // MSPART Partition driver support
    if "%SYSGEN_MSPART%"=="1"    set SYSGEN_STOREMGR=1
    if "%SYSGEN_MSPART%"=="1"    set CE_MODULES=%CE_MODULES% mspart

    REM ==============================================================================================
    REM FSDMGR/STOREMGR
    REM ==============================================================================================

    if "%SYSGEN_STOREMGR%"=="1"  set CE_MODULES=%CE_MODULES% fsdmgr
    if "%SYSGEN_STOREMGR%"=="1"  set SYSGEN_MSGQUEUE=1
    if "%SYSGEN_STOREMGR%"=="1"  set __SYSGEN_FILESYS=1
    if "%SYSGEN_STOREMGR%"=="1"  set SYSGEN_STDIO=1


    REM ==============================================================================================
    REM
    REM Optional components from core OS
    REM
    REM ==============================================================================================

    REM ZLIB APIs for doing decompression, mainly used for PNG files
    if "%__SYSGEN_ZLIB%"=="1" set CE_MODULES=%CE_MODULES% zlib

    if "%SYSGEN_FIBER%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% fiber

    if "%BSP_FPEMUL%"=="1" set __SYSGEN_FPEMUL=1

    if "%__SYSGEN_FPEMUL%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% fpemul


    if not "%SYSGEN_IMM%"=="1" goto No_IMM
        REM // Traditional Chinese IM's
        if "%SYSGEN_CHAJEIIM%"=="1" set CE_MODULES=%CE_MODULES% chajeiim
        if "%SYSGEN_PHONIM%"=="1" set CE_MODULES=%CE_MODULES% phonim
    :No_IMM

    if "%SYSGEN_IMM%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% coreimm
    if not "%SYSGEN_IMM%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% coreimmstub

    if "%SYSGEN_GB18030%"=="1" set CE_MODULES=%CE_MODULES% gb18030
    if "%SYSGEN_GB18030%"=="1" set __SYSGEN_NEED_LOCUSA=1

    REM // Handwriting recognition
    if "%SYSGEN_MBOXKOR%"=="1" set SYSGEN_HWX=1
    if "%SYSGEN_MBOXCHT%"=="1" set SYSGEN_HWX=1

    if not "%SYSGEN_HWX%"=="1" goto nohwx
    set SYSGEN_FULL_CRT=1
    if not "%LOCALE%" == "0411" goto Not_J_HWX
        set CE_MODULES=%CE_MODULES% hwxjpn
        goto nohwx
    :Not_J_HWX

    if not "%__SYSGEN_CHINESE_TRADITIONAL%"=="1"  goto Not_CHT_HWX
        set CE_MODULES=%CE_MODULES% hwxcht
        if "%SYSGEN_MBOXCHT%"=="1" set CE_MODULES=%CE_MODULES% mboxcht
        goto nohwx
    :Not_CHT_HWX

    if not "%LOCALE%" == "0412" goto Not_K_HWX
        set CE_MODULES=%CE_MODULES% hwxkor
        if "%SYSGEN_MBOXKOR%"=="1" set CE_MODULES=%CE_MODULES% mboxkor
        goto nohwx
    :Not_K_HWX

    REM For all other locales include the US HWX component.
    set CE_MODULES=%CE_MODULES% hwxusa
    set SYSGEN_NKMAPFILE=1

    :nohwx

    if "%SYSGEN_FMTRES%"=="1"     set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% fmtres
    if "%SYSGEN_FMTRES%"=="1"     set SYSGEN_FMTMSG=1
    if "%SYSGEN_FMTMSG%"=="1"     set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% fmtmsg
    if "%SYSGEN_CORESTRA%"=="1"   set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% corestra
    if "%SYSGEN_STDIO%"=="1"      set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% coresiow
    if "%SYSGEN_STDIOA%"=="1"     set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% coresioa
    if "%SYSGEN_STDIO%"=="1"      set SYSGEN_SERDEV=1
    if "%SYSGEN_STDIOA%"=="1"     set SYSGEN_SERDEV=1
    if "%SYSGEN_SERDEV%"=="1"  set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% serdev


    if "%SYSGEN_FSRAMROM%"=="1"    set __SYSGEN_FILESYS=1
    if "%SYSGEN_FSRAMROM%"=="1"    set SYSGEN_FSROMONLY=
    if "%SYSGEN_FSROMONLY%"=="1"   set __SYSGEN_FILESYS=1

    if "%SYSGEN_FSREGRAM%"=="1"    set __SYSGEN_FILESYS=1
    if "%SYSGEN_FSREGRAM%"=="1"    set SYSGEN_FSREGHIVE=
    if "%SYSGEN_FSREGHIVE%"=="1"   set __SYSGEN_FILESYS=1

    if "%SYSGEN_FSREPLBIT%"=="1"   set __SYSGEN_FILESYS=1
    if "%SYSGEN_FSREPLBIT%"=="1"   set SYSGEN_FSREPLCOUNT=
    if "%SYSGEN_FSREPLCOUNT%"=="1" set __SYSGEN_FILESYS=1
    if "%SYSGEN_ACL%"=="1"         set SYSGEN_ACCOUNTDB=1
    if "%SYSGEN_ACCOUNTDB%"=="1"   set SYSGEN_FSDBASE=1

    if "%SYSGEN_FSDBASE%"=="1"     set __SYSGEN_FILESYS=1
    if "%SYSGEN_FSPASSWORD%"=="1"  set __SYSGEN_FILESYS=1
    if "%SYSGEN_FSADVERTISE%"=="1"  set __SYSGEN_FILESYS=1
    if "%SYSGEN_FSADVERTISE%"=="1"  set SYSGEN_MSGQUEUE=1
    if "%SYSGEN_MSGQUEUE%"=="1"    set __SYSGEN_FILESYS=1

    if "%SYSGEN_ACL%"=="1"         set __SYSGEN_FILESYS=1
    if "%SYSGEN_ACL%"=="1"         set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% acl


    if not "%__SYSGEN_FILESYS%"=="1"   goto noFileSys
        set CE_MODULES=%CE_MODULES% filesys
        set FILESYS_COMPONENTS=fsheap fsmain
        set SYSGEN_FULL_CRT=1
        set SYSGEN_NKMAPFILE=1
        set SYSGEN_NKCOMPR=1
        set SYSGEN_STRSAFE=1

        REM Choice of registry: cannot choose both, fsregram is default and wins ties
        if not "%SYSGEN_FSREGHIVE%"=="1"   set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fsreg
        if "%SYSGEN_FSREGHIVE%"=="1"       set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fsreghive

        REM Choice of internal file system: cannot choose both, fsramrom is default and wins ties
        if not "%SYSGEN_FSROMONLY%"=="1"        set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fsysram
        if "%SYSGEN_FSROMONLY%"=="1"       set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fsysrom

        REM Choice of replication: cannot choose both, fsreplbit wins ties
        if "%SYSGEN_FSREPLCOUNT%"=="1"     set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fsreplcount
        if "%SYSGEN_FSREPLBIT%"=="1"       set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fsreplbit

        REM Other components: database, password, advertise interface, msgqueue are independent
        if "%SYSGEN_FSDBASE%"=="1"         set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fsdbase
        if "%SYSGEN_FSDBASE%"=="1"         set __SYSGEN_NEED_LOCUSA=1
        if "%SYSGEN_FSPASSWORD%"=="1"      set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fspass
        if "%SYSGEN_FSADVERTISE%"=="1"     set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fsadvertise
        if "%SYSGEN_MSGQUEUE%"=="1"        set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% msgqueue
        if "%SYSGEN_ACCOUNTDB%"=="1"       set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% adb
        if "%SYSGEN_ACL%"=="1"             set FILESYS_COMPONENTS=%FILESYS_COMPONENTS% fsacl
    :noFileSys

    REM Kernel components
    if "%SYSGEN_NKCOMPR%"=="1"     set NK_COMPONENTS=%NK_COMPONENTS% nkcompr
    if not "%SYSGEN_NKCOMPR%"=="1" set NK_COMPONENTS=%NK_COMPONENTS% nknocomp
    if "%SYSGEN_NKMAPFILE%"=="1"       set NK_COMPONENTS=%NK_COMPONENTS% nkmapfile
    if not "%SYSGEN_NKMAPFILE%"=="1"   set NK_COMPONENTS=%NK_COMPONENTS% nknomapfile

    REM If FileSys is not selected then they can include TKTest
    if not "%__SYSGEN_FILESYS%"=="1" if "%SYSGEN_TKTEST%"=="1" set CE_MODULES=%CE_MODULES% tktest

    REM By default the config get's LocMini support
    REM Optionally you can add LOCUSA (US only code page support) or CORELOC (full multi-codepage support)
    rem Did someone need at least LOCUSA support?
    if not "%LOCALE%"=="0409" set SYSGEN_CORELOC=1
    if "%SYSGEN_CORELOC%"=="1" set SYSGEN_LOCUSA=
    if "%__SYSGEN_NEED_LOCUSA%"=="1" if not "%SYSGEN_CORELOC%"=="1" set SYSGEN_LOCUSA=1

    if "%SYSGEN_CORELOC%"=="1"     set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% coreloc
    if not "%SYSGEN_CORELOC%"=="1" if "%SYSGEN_LOCUSA%"=="1"     set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% locusa
    if not "%SYSGEN_CORELOC%"=="1" if not "%SYSGEN_LOCUSA%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% locmini

    REM MUI APIs are part of coredll all the time, but MUI will get enabled ony if SYSGEN_MULTIUI is set
    set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% multiui

    REM Include CPP EH and runtime type information (RTTI) support
    if "%SYSGEN_CPP_EH_AND_RTTI%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% crt_cpp_eh_and_rtti

    REM This will include the STRSAFE component
    if not "%SYSGEN_STRSAFE%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% nostrsafe

    REM Toolhelp api's for getting system information
    if "%SYSGEN_TOOLHELP%"=="1" set CE_MODULES=%CE_MODULES% toolhelp

    REM VMINI driver allows product TCP/IP to work over debug Ethernet
    if "%SYSGEN_ETHERNET%"=="1" set CE_MODULES=%CE_MODULES% vmini



    REM LASS is the local authentication Subsystem.
    if "%SYSGEN_LASS%"=="1"  set CE_MODULES=%CE_MODULES% lassd
    if "%SYSGEN_LAP_PSWD%"=="1" set CE_MODULES=%CE_MODULES% lap_pw

    REM ==============================================================================================
    REM
    REM More extensive C-Runtime support.  Needed by some components and potentially customer apps
    REM
    REM ==============================================================================================
    if "%SYSGEN_FULL_CRT%"=="1" set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% full_crt

    if "%__SYSGEN_FILESYS%"=="1" set __SYSGEN_CRYPTHASH=1
    if "%__SYSGEN_CRYPTHASH%"=="1"  set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% crypthash rsa32
    REM These test programs require FileSys
    if "%__SYSGEN_FILESYS%"=="1" set CE_MODULES=%CE_MODULES% rt_tests

    if not "%CE_EXTRA_MODULES%"==""         set CE_MODULES=%CE_MODULES% %CE_EXTRA_MODULES%

    REM Fix things in other trees as necessary

    goto :EOF
:Not_Pass1
if /i not "%1"=="pass2" goto :Not_Pass2
    goto :EOF
:Not_Pass2
if /i not "%1"=="report" goto :Not_Report
    echo CE_MODULES=%CE_MODULES%
    echo COREDLL_COMPONENTS=%COREDLL_COMPONENTS%
    echo NK_COMPONENTS=%NK_COMPONENTS%
    if not "%FILESYS_COMPONENTS%"=="" echo FILESYS_COMPONENTS=%FILESYS_COMPONENTS%
    if not "%DEVICE_COMPONENTS%"=="" echo DEVICE_COMPONENTS=%DEVICE_COMPONENTS%
    if not "%PM_COMPONENTS%"==""    echo PM_COMPONENTS=%PM_COMPONENTS%
    if not "%FATUTIL_COMPONENTS%"=="" echo FATUTIL_COMPONENTS=%FATUTIL_COMPONENTS%
    if not "%GWE1_COMPONENTS%"=="" echo GWE1_COMPONENTS=%GWE1_COMPONENTS%
    if not "%GWE2_COMPONENTS%"=="" echo GWE2_COMPONENTS=%GWE2_COMPONENTS%
    if not "%GWE3_COMPONENTS%"=="" echo GWE3_COMPONENTS=%GWE3_COMPONENTS%
    if not "%GWE4_COMPONENTS%"=="" echo GWE4_COMPONENTS=%GWE4_COMPONENTS%
    if not "%ATAPI_COMPONENTS%"=="" echo ATAPI_COMPONENTS=%ATAPI_COMPONENTS%
    goto :EOF
:Not_Report
echo %0 Invalid parameter %1

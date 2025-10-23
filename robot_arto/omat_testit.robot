*** Settings ***
Library   String
Library   SerialLibrary

#Test Setup      Connect Serial
#Test Teardown   Disconnect Serial

*** Variables ***
${com}   	COM4  #Vaihdoin omaan, muista vaihtaa takas!
${baud} 	115200
${board}	nRF

#inputs
${seq}      000010X
${ryg}      ARYGX
${str}     000069X

${RED}      R1000X
${GREEN}    G1000X
${YELLOW}   Y500X

#Results
${correcttimer}    10X
${incorrecttimer}  -3X
${debug}    DX
${debugresponse}    "DEBUGGING ENABLED"

*** Keywords ***
Connect Serial
    Log To Console  Connecting to ${board}
    Add Port  ${com}  baudrate=${baud}  encoding=ascii
    Port Should Be Open  ${com}
    Reset Input Buffer
    Reset Output Buffer

Disconnect Serial
    Log To Console  Disconnecting ${board}
    Delete Port  ${com}

Reset Buffers
    Reset Input Buffer
    Reset Output Buffer
    Log To Console  --- Buffers Reset ---

*** Test Cases ***

Connect Serial
    Log To Console  Connecting to ${board}
    Add Port  ${com}  baudrate=${baud}  encoding=ascii
    Port Should Be Open  ${com}
    Reset Input Buffer
    Reset Output Buffer



Serial Set Leds
    # Lähetetään arvot ledeille R1000, Y500, G1000 ILMAN PILKKUA, vastaus on -3, mutta ledit vaihtuu ajastukselle
    Write Data  ${RED}   encoding=ascii
    Sleep   3s
    Write Data  ${YELLOW}   encoding=ascii
    Sleep   3s
    Write Data  ${GREEN}  encoding=ascii
    Sleep   3s
    Log To Console  Values set to leds: Not really a test.
    Sleep   3s

Serial Led Control
    Reset Buffers
    # vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
	#${read} =   Read Until   terminator=58   encoding=ascii 
    #vai 58?

    # lähetetään RYG ja lopetusmerkki X
    Write Data   ${ryg}   encoding=ascii 
    Log To Console   Send sequence  ${ryg}

    # vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
    ${read} =   Read Until   terminator=58   encoding=ascii 
    #vai 58?

    # konsolille näkyviin vastaanotettu merkkijono
    Log To Console   Received ${read}
	
    # vertaillaan merkkijonoa
    #Should Be Equal As Strings   ${read}    ${correcttimer}
    #Log To Console   Tested ${read} is same as ${correcttimer}

    # astetta hankalampi tehdä testaus numeroina
    # koska lopetusmerkki X pitää ensin poistaa merkkijonosta
    # tai vaihtaa lopetusmerkki esim \0
    # Should Be Equal As Integers   ${read}    -1
    
    Sleep  8s

Timeout incorrect
    # lähetetään 000069 ja lopetusmerkki X
    Write Data   ${str}   encoding=ascii 
    Log To Console   Send sequence ${str}

    # vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
	${read} =   Read Until   terminator=58   encoding=ascii  
    #vai 58?

    # konsolille näkyviin vastaanotettu merkkijono
    Log To Console   Received ${read}

    # vertaillaan merkkijonoa
	Should Be Equal As Strings  ${read}  ${incorrecttimer}
    Log To Console  Tested ${read} is same as ${incorrecttimer}

    sleep   4s

Timeout Correct
    # lähetetään 000010 ja lopetusmerkki X
    Write Data   ${seq}   encoding=ascii 
    Log To Console   Send sequence ${seq}

    # vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
    ${read} =   Read Until   terminator=58   encoding=ascii 
    #vai 58?

    # konsolille näkyviin vastaanotettu merkkijono
    Log To Console   Received ${read}
	
    # vertaillaan merkkijonoa
    Should Be Equal As Strings   ${read}    ${correcttimer}
    Log To Console   Tested ${read} is same as ${correcttimer}

    Sleep  12s

Traffic Sequence Correct Short
    # lähetetään toimiva "RYG"
    ${sequenceryg}	Set Variable	ARYGX
    Write Data	${sequenceryg}   encoding=ascii
    ${read} = 	Read Until   terminator=58   encoding=ascii 
    #vai 58?
    Should Be Equal As Strings    ${read}    RYGX
	Log To Console    Sent ${sequenceryg}, received ${read}
	
Traffic Sequence Incorrect Short
    Reset Buffers
    # lähetetään väärä "RjG"
    ${seq}      Set Variable    ARjGX
    ${firstresult}  Set Variable    RGX
    Write Data  ${seq}   encoding=ascii
    ${read} =    Read Until   terminator=58   encoding=ascii 
    #vai 58?
    Log To Console    Sent ${seq}, received ${read}
    Should Be Equal As Strings    ${read}    ${firstresult}

Traffic Sequence Correct Long
    Reset Buffers
    # lähetetään toimiva "RYGRYGRYG"
    ${seq}      Set Variable    ARYGRYGRYGX
    ${secondresult}     Set Variable    RYGRYGRYGX
    Write Data  ${seq}   encoding=ascii
    ${read} =    Read Until   terminator=58   encoding=ascii 
    #vai 58?
    Log To Console    Sent ${seq}, received ${read}
    Should Be Equal As Strings    ${read}    ${secondresult}

Traffic Sequence Incorrect Long
    Reset Buffers
    # lähetetään väärä "RYGrRYGxG"
    ${seq}      Set Variable    ARYGrRYGxGX
    ${thirdresult}  Set Variable    RYGRRYGX
    Write Data  ${seq}   encoding=ascii
    ${read} =    Read Until   terminator=58   encoding=ascii 
    #vai 58?
    Log To Console    Sent ${seq}, received ${read}
    Should Be Equal As Strings    ${read}    ${thirdresult}

Traffic Sequence Empty
    Reset Buffers
    # lähetetään tyhjä "X"
    ${seq}      Set Variable    X
    ${fourthresult}     Set Variable    -3X
    Write Data  ${seq}   encoding=ascii
    ${read} =    Read Until   terminator=58   encoding=ascii 
    #vai 58?
    Log To Console    Sent ${seq}, received ${read}
    Should Be Equal As Strings    ${read}    ${fourthresult}

Disconnect Serial
    Log To Console  Disconnecting ${board}
    [TearDown]  Delete Port  ${com}

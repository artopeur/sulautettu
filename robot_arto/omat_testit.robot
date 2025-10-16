*** Settings ***
Library   String
Library   SerialLibrary

*** Variables ***
${com}   	COM4  #Vaihdoin omaan, muista vaihtaa takas!
${baud} 	115200
${board}	nRF

#inputs
${seq}      000010X
${ryg}      RYGX
${str}     000069X

${RED}      R1000X
${GREEN}    G1000X
${YELLOW}   Y500X

#Results
${correcttimer}    10X
${incorrecttimer}  -3X
${debug}    DX
${debugresponse}    "DEBUGGING ENABLED"

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
    # vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
	#${read} =   Read Until   terminator=58   encoding=ascii 

    # lähetetään RYG ja lopetusmerkki X
	Write Data   ${ryg}   encoding=ascii 
	Log To Console   Send sequence  ${ryg}

	# vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
	${read} =   Read Until   terminator=58   encoding=ascii 

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

	# konsolille näkyviin vastaanotettu merkkijono
	Log To Console   Received ${read}
	
	# vertaillaan merkkijonoa
	Should Be Equal As Strings   ${read}    ${correcttimer}
	Log To Console   Tested ${read} is same as ${correcttimer}

    Sleep  12s

Disconnect Serial
	Log To Console  Disconnecting ${board}
	[TearDown]  Delete Port  ${com}

//Jimmy Abruzzzese, Marissa Boucher, Nick Almeder

int strCompare(char*, char*);
void parseCommand(char*);
void empty(char* command, int);

main()
{
	char command[100];

	enableInterrupts();
	while(1)
	{
		syscall(0, "\r\n");
		syscall(0, "Shell:>");
		syscall(1, command);
		parseCommand(command);
		//syscall(0, "\r\n");
		//syscall(0, command);
		//syscall(0, "\r\n");
		empty(&command, 100);
		//syscall(0, command);
		//if (strCompare(command, "type"))
		//	syscall(0, "Filler");
		//else
		//	syscall(0, "Bad Command");

	}
}

int strCompare(char* str1, char* str2)
{
        int i;
        for(i = 0; i<6; i++)
        {
                if(str1[i] != str2[i])
                        return 0;
                if(str1[i] == '\0' && str2[i] == '\0')
                        break;
                if(str1[i] == '\r' && str2[i] == '\0')
                        break;
        }
        return 1;
}


void parseCommand(char* command)
{
	int i;
	char parsedCommand[100];
	int counter = 0;
	char parsedObject[100];
	int j =0;
	int k =0;
	int h = 0;
	char copyFile[100];
	char primeFile[100];
	int lineNum = 0;
	char file[13312];
	char typed[100];
	char newTXT [512];
	int sectorsRead = 0;
	int check = 0;
	int keep_typing = 1;
	for (i = 0; i<100; i++)
	{
		if (command[i] == "\0" || command[i] == 0x20)
		{
			break;
		}
		else
		{
			parsedCommand[i] = command[i];
			counter++;
		}
	}
	counter = counter+1;
	for (i = counter; i<100; i++)
	{
		if (command[i] == "\0")
		{
			break;
		}
		else
		{
			parsedObject[j++] = command[i];
		}
	}
	//syscall(0, parsedCommand);
	//syscall(0, parsedObject);
	if (strCompare(parsedCommand, "type"))
	{
		empty(&file, 13312);
		syscall(0, "\r\n");
		syscall(3, file, parsedObject, &sectorsRead);
		if (file[1] != 0x0)
		{
			syscall(0, file);
		}
		else
		{
			syscall(0, "File not found");
		}
	}
	if (strCompare(parsedCommand, "exec"))
	{
		syscall(3, file, parsedObject, &sectorsRead);
		if (sectorsRead  != 0x0)
		{
			//syscall(0, parsedObject);

			syscall(4, parsedObject);
		}
		else
		{
			syscall(0, "Program not found");
		}
		//empty(&file, 13312);
	}
	if (strCompare(parsedCommand, "dir"))
	{
		syscall(7,0,0,0);
	}

	if (strCompare(command, "create textfl"))
	{
		i = 0;
		j = 0;
		k = 0;
		while (keep_typing)
		{
			empty(typed, 100);
			syscall(0, "Input text: ");
			syscall(1, typed);
			syscall(0, "\r\n");
			if (typed[0] == 0x0)
			{
				break;
			}
			else
			{
				for (i = 0; i<100; i++)
				{
					if (typed[i] == 0x0 || typed[i] == '\0' || typed[i] == '\r' || typed[i] == '\n' || typed =='\t')
					{
						typed[i] = 0x20;
					}
					newTXT[i+100*j] = typed[i];
				}
				j++;
				lineNum++;
			}
		}
		syscall(9, newTXT, "textfl",lineNum);
		//syscall(0, newTXT);
	}

	if (strCompare(parsedCommand, "kill"))
	{
		//syscall(0,"Parsed Object is here:");
		//syscall(0,parsedObject);
		syscall(10,parsedObject-0x30,0,0);
	}
}


void empty(char* command, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		if (command[i] == 0x0)
			break;
		command[i] = 0x0;
	}
}

//Jimmy Abruzzese, Nick Almeder, Marissa Boucher

void printString(char*);
void readSector(char*, int);
void writeSector(char*, int);
void dir();
void handleInterrupt21(int,int,int,int);
void readString(char*);
void printChar(char);
void deleteFile(char*);
void writeFile(char*, char*, int);
int strCompare(char* str1, char* str2);
void readFile(char *buff,char *name, int *sectorsRead);
void executeProgram(char* name);
void terminate();
void handleTimerInterrupt(int segment, int sp);
void killProcess(int process);

int processActive[8];
int processStackPointer[8];
int currentProcess;

void main() {
	int i;


	for(i=0;i<8;i++)
	{
		processActive[i] = 0;
		processStackPointer[i] = 0xff00;
	}
	currentProcess = -1;


	makeInterrupt21();
	interrupt(0x21,4,"shell",0,0);

	makeTimerInterrupt();
	while(1);
}

void printString(char* chars){
	int index = 0;
	while (chars[index] != '\0')
	{
		int character = chars[index];
		interrupt(0x10, 0xe*256+character, 0, 0, 0);
		index++;
	}

}


void readSector(char* buffer, int sector){
	int rel_sec = sector+1;
	interrupt(0x13,2*256+1,buffer,0*256+rel_sec, 0*256+0x80);
}
void writeSector(char* buffer, int sector)
{
	int rel_sec = sector+1;
	interrupt(0x13,3*256+1,buffer,0*256+rel_sec, 0*256+0x80);
}


void handleInterrupt21(int ax, int bx, int cx, int dx){
	if (ax == 0)
	{
		printString(bx);
	}
	else if (ax == 1)
	{
		readString(bx);
	}
	else if (ax == 2)
	{
		readSector(bx, cx);
	}
	else if (ax == 3)
	{
		readFile(bx,cx,dx);
	}
	else if (ax ==4)
	{
		executeProgram(bx);
	}
	else if (ax ==5)
	{
		terminate();
	}
	else if (ax ==6)
	{
		writeSector(bx,cx);
	}
	else if(ax ==7)
	{
		dir();
	}
	else if(ax ==8)
	{
		handleTimerInterrupt(bx,cx);
	}
	else if(ax == 8)
	{
		deleteFile(bx);
	}
	else if(ax == 9)
	{
		writeFile(bx,cx,dx);
	}
	else if(ax == 10)
	{
		killProcess(bx);
	}
	else
	{
		printString("You cant handle the interrupt!");
	}
}


void printChar(char print)
{
	char* p[2];
	p[0] = print;
	p[1] = "\0";
	printString(p);
}

void readString(char* chars)
{
	int enter = 0xd;
	int lineFeed = 0xa;
	int endOfString = 0x0;
	int backspace = 0x8;

	int end = 1;
	int count = 0;
	while(end)
	{
		int c = interrupt(0x16,0,0,0,0);
		if(c  == enter)
		{
			chars[count] = endOfString;
			chars[count+1] = lineFeed;
			chars[count+2] = enter;
			printChar(endOfString);
			printChar(lineFeed);
			printChar(enter);
			end = 0;
		}
		else if(c == backspace)
		{
			if(count > 0)
			{
				chars[count] = endOfString;
				count --;
				interrupt(0x10,0xe*256 + 0x8,0,0,0);
				count ++;
				interrupt(0x10,0xe*256 + 0x0,0,0,0);
				count --;
				interrupt(0x10,0xe*256 + 0x8,0,0,0);
			}
		}
		else
		{
			chars[count]=c;
			printChar(chars[count]);
			count++;
		}
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

void readFile(char *buff, char *name, int *sectorsRead)
{
	char dir[512];
	int i;
	int j;
	int file;
	int sectors[27]; /*0-25, 26 being \0*/
	int bufferspace=0;
	*sectorsRead=0;
	readSector(dir,2);
	/*the loop below could also be from 0-16, and if the file is found
	multiply i*32.*/
	for(i=0;i<512;i+=32)
	{
		file = strCompare(dir+i,name);/* or dir+i*32 */
		if(file != 0)
		{
			sectors[26] = 0; /*0-25 may be filled, but 26
					must be the end, as 26+6 = 32 which
					is the maximum amount of sectors
					available.*/
			for(j=0;j<26;j++)
			{
				/*loop through sector at i*32
				skip the first 6, which contains the name*/
				if(dir[(i+6)+j]==0)
					break;
				else
				{
					/*readsector where BX=the ACTUAL buffer
					and CX = the temporary 'dir' buffer
					locally initialized.
					bufferspace+=512 was a given instruction*/
					readSector(buff+bufferspace,dir[(i+6)+j]);
					bufferspace +=512;
					*sectorsRead = *sectorsRead + 1;
				}
			}
					 /*pointer to sectorsRead to relay
					  information back to main()*/
		}
	}
}

void dir()
{
	int i;
	int j;
	int k;
	char buff[512];
	char out[8];

	readSector(buff,2);

	/*loop through sectors just like readfile*/
	for(i=0;i<512;i+=32)
	{
		if(buff[i] != '\0')
		{
			/*if something is found, put the name in out[]*/
			for(j=0;j<6;j++)
			{
				out[j]=buff[i+j];
			}
				/*formatting*/
			out[6]='\r';
			out[7]='\n';
			for(k=0;k<8;k++)
			{
				printChar(out[k]);
			}
		}

	}

}

void executeProgram(char* name)
{
	int i=0;
	int j=0;
	int segment = 0x2000;
	char buffer[13312];
	int sectorsRead;
	int dataSeg;

//	makeInterrupt21();

	//step 3-1, read buffer
	interrupt(0x21, 3, buffer, name, &sectorsRead);
	//readFile(buffer, name, &sectorsRead);

	//step 3-2, find free entry
	dataSeg = setKernelDataSegment();

	while(j<8 && processActive[j]!=0)
	{
		j++;
	}
	restoreDataSegment(dataSeg);

	//step 3-3 add 2 x 0x1000, 0 == 0x2000, 1 == 0x3000 ... 7 == 0x09000
	segment = (j+2)*0x1000;

	//step 3-4 copy buffer, put in memory
	for (i = 0; i < 13312; i++)
	{
		putInMemory(segment, i, buffer[i]);
	}

	//step 3-5 initialize (prints i-n when i-nitialized)
	initializeProgram(segment);

	//step 3-6 set processActive to 1
	dataSeg = setKernelDataSegment();
	processActive[j] = 1;
	processStackPointer[j] = 0xff00;
	restoreDataSegment(dataSeg);
}


void deleteFile(char* fileName)
{
	char dir[512];
	char map[512];
	int sectors[27];
	int i;
	int j = 0;
	int mapSector;
	char dirFileName[6];
	//Load the directory and map to the character arrays
	printChar('a');
	readSector(map, 1);
	readSector(dir, 2);

	//search to see if the file name is in the directory
	for (i = 32; i<38; i++)
	{
		dirFileName[j++] = dir[i];
		//printChar(dirFileName[j-1]);
		//printChar(dir[i]);
	}
	for (i = 0; i<6; i++)
	{
		printChar(fileName[i]);
	}
	if (strCompare(dirFileName, fileName) == 0)
	{
		printChar('q');
	}
	if (strCompare(dirFileName, fileName))
	{
	printChar('b');
	//if found  set the first byte of the  file name to 0
		dir[32] = 0x00;
		// set fill an array with the hex values after the filename and then set all those values to 0
		for(i=0; i<26; i++)
		{
			sectors[i] = dir[38+i];
			dir[38+1] = 0x00;
		}
		sectors[26] = 0x00;
		//step through the sectors listed belonging to the file and set the corresponding map byte to \0. 
		for(i=0; i<25; i++)
		{
			mapSector = sectors[i];
			if (mapSector == 0)
			{
				break;
			}
			map[mapSector] = 0x00;
		}
	}
	else
	{
		// if not found print an error message
		printString("File not found\n");
	}
	printChar('c');
	//write the map and sector arrays back to the memory
	writeSector(map,1);
	writeSector(dir,2);
}

void writeFile(char* buffer, char* fileName, int numberOfSectors)
{
	char map[512];
	char dir[512];
	int i;
	int j;
	int num;
	int dirSector;
	int sectorFound = 0;
	int sect;
	char buff[512];
	readSector(map,1);
	readSector(dir,2);
	for(dirSector = 0; dirSector < 16; dirSector++)
	{
		if (dir[dirSector*32] == 0x00)
		{
			sectorFound = 1;
			break;
		}
	}
	if (sectorFound == 0)
	{
		return;
	}

	for (i = 0; i <6; i++)
	{
		printChar(fileName[i]);
		if (fileName[i] == 0x00)
		{
			dir[dirSector*32+i] = '\0';
		}
		else
		{
			dir[dirSector*32+i] = fileName[i];
		}
	}

	for (i=0; i < numberOfSectors; i++){
		sect = 4;
		while(map[sect] != 0x0){
			sect++;
		}
		if (sect == 26)
		{
			return;
		}
		map[sect] = 0xFF;

		dir[32*dirSector+6+i] = sect;
		/*for(j = 0; j<512; j++)
		{
			num = i+1;
			buff[j] = buffer[j*num];
		}*/
		writeSector(buffer, sect);
	}


	writeSector(map,1);
	writeSector(dir,2);
}


void terminate()
{
/*	char shellname[6];
	shellname[0] = 's';
	shellname[1] = 'h';
	shellname[2] = 'e';
	shellname[3] = 'l';
	shellname[4] = 'l';
	shellname[5] = '\0';
	executeProgram(shellname);*/
	int dataSeg;
	dataSeg = setKernelDataSegment();
	processActive[currentProcess] = 0;
	restoreDataSegment(dataSeg);

	while(1);
}

void handleTimerInterrupt(int segment, int sp)
{
	int i;
	int dataSeg;

	//step 4-1
	dataSeg = setKernelDataSegment();

	//step 4-2 find pointer sp indexed by current process
	if(currentProcess != -1)
	{
		processStackPointer[currentProcess] = sp;
	}

	//add one to currentProcess until active process found
	//when this breaks there should be an active process...should

	//step 4-3 - find active process round-robin
	while(1)
	{
		currentProcess +=1;
		if(currentProcess == 8)
			currentProcess = 0;
		if(processActive[currentProcess] == 1)
		{
			break;
		}
	}

	//step 4-4 set segment
	segment = (currentProcess+2)*0x1000;
	sp = processStackPointer[currentProcess];


	//optional step: display active process at top of screen
	for(i=0; i<8; i++)
        {
                putInMemory(0xb800,60*2+i*4,i+0x30);
                if(processActive[i]==1)
                        putInMemory(0xb800,60*2+i*4+1,0x20);
                else
                        putInMemory(0xb800,60*2+i*4+1,0);
        }

	//step 4-5 restore datasegment
	restoreDataSegment(dataSeg);


	returnFromTimer(segment, sp);
}

void killProcess(int process)
{
	int dataSeg;
	dataSeg = setKernelDataSegment();
	processActive[process] = 0;
	restoreDataSegment(dataSeg);

}

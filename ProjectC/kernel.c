//Jimmy Abruzzese, Nick Almeder, Marissa Boucher

void printString(char*);
void readSector(char*, int);
void handleInterrupt21(int,int,int,int);
void readString(char*);
void printChar(char);
int strCompare(char* str1, char* str2);
void readFile(char *buff,char *name, int *sectorsRead);

void main() {
	char buffer[13312]; /* 512*(32-6) */
	int sectorsRead;
	readFile(buffer,"messag",&sectorsRead);
	if(sectorsRead > 0)
		printString(buffer);
	else
		printString("Error: No such sector.");
	/* This is for testing later

	makeInterrupt21();

	interrupt(0x21, 3, "message.txt", buffer, &sectorsRead);
	if(sectorsRead > 0)
		interrupt(0x21,0,buffer,0,0);
	else
		interrupt(0x21,0,"Message not found\r\n",0,0);*/

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
		readFile(bx,cx);
	}
	else
	{
		printString("You cant handle the interrupt!");
	}
}

//Nick's code

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
	int bufferspace;
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
				}
			}
			*sectorsRead +=1; /*pointer to sectorsRead to relay
					  information back to main()*/
		}
	}
}

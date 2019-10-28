void printString(char* chars);
void printChar(char print);
void readString(char* chars);

void main()
{
	char line[80];
	printString("Enter a string: ");
	readString(line);
	printString(line);

	while(1);
}

void printString(char* chars)
{
	int i = 0;
	while(chars[i] != 0x0)
	{
		interrupt(0x10,0xe*256 + chars[i],0,0,0);
		i++;
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


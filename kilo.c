#include<stdio.h>
#include<ctype.h>
#include<termios.h>
#include<unistd.h>
#include<stdlib.h>
/*raw mode sets up the TTY driver
to pass every character to the program as it is typed.
Programs (on Unixish operating systems) are started in cooked mode by default
and need to enable raw mode
*/
struct termios original_attributes;//copies original attributes into a a variable
void disablerawmode(){
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&original_attributes);
    //sets back the original attributes 
}


/*In order to enable raw mode without using some external library
like ncurses(which abstracts all these things for us)
we can do following:
1. using tcgetattr() to read the current attributes into a struct
2.modifying the struct by hand
3.passing the modified struct to tcsetattr() to write the new terminal attributes back out
this way we can turn off the echoing of output after the program exit
for more information:https://c-for-dummies.com/blog/?p=7350*/


void enablerawmode(){



//struct termios, tcgetattr(), tcsetattr(), ECHO, and TCSAFLUSH all come from <termios.h>.
/*here is an explanation of all of these
tcgetattr():gets the parametters associated with the terminal
tcsetattr():sets atributes to the terminal
ECHO:printing out on the terminal we wanna disable this
TCSAFLUSH argument specifies when to apply the change:
 in this case, it waits for all pending output to be written to the terminal,
and also discards any input that hasn’t been read.*/



    tcgetattr(STDIN_FILENO, &original_attributes);
    atexit(disablerawmode);//calls the function disablerawmode() automatically at exit

    struct termios raw;//termios is a struct declared inside of termios.h

    tcgetattr(STDIN_FILENO, &raw);//gets the current attributes into raw
    
    //atexit provided by stdlib

    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);//disables echo,canonical mode and ctrl + c (SIGINT) and ctrl +z (SIGTSTP) signals
    //IEXTEN disables ctrl + v and it also disables crl + o on macos
    //uses ICANON flag to disable cooked mode
    //don't quite get it how it works but basically it compliments the ECHO
    //and then stores it inside of raw.c_lflag to revert it's behaviour
    //man it's some low level shit which i am unable to wrap my head around
    raw.c_iflag &= ~(BRKINT | ICRNL | IXON | INPCK | ISTRIP);//ICRNL fixes ctrl + m
    //BRKINT, INPCK, ISTRIP, and CS8 all come from <termios.h> and disable some miscallenous flags behaviours
    //they might be irrelevant for modern terminals
    /*By default, Ctrl-S and Ctrl-Q are used for software flow control. Ctrl-S stops data from being transmitted to the terminal until you press Ctrl-Q
    IXON comes from <termios.h>. The I stands for “input flag” (which it is, unlike the other I flags we’ve seen so far) and XON comes from the names 
    of the two control characters that Ctrl-S and Ctrl-Q produce: XOFF to pause transmission and XON to resume transmission.*/
     raw.c_cflag |= (CS8);
    raw.c_oflag &= ~(OPOST);//disbales any kind of output specially needed for diabling of carriage return
    //which was ctrl + m, we already disbaled it for our input and now this line disbles it for output as well
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    //after changing the flag it writes it
    //god vim is so much faster, vscode is a pain 
}


int main(){
    enablerawmode();
    char c;//variable to read characters from users

    /*read() and STDIN_FILENO come from <unistd.h>.
    We are asking read() to read 1 byte from the standard input into the variable c
    and keep reading it until the eof has reached
    read() returns the number of bytes that it read
    and returns 0 when end of file is reached*/
    
    while (read(STDIN_FILENO, &c, 1) ==1 && c!='q'){
        //iscntrl comes from ctype and checks if the character is from control characters
        //control characters are non printable asccii characters refer nto ASCII table for more info
        //if it is one of the control charcters it prints its decimal
        if (iscntrl(c))
            printf("%d\r\n",c);

        else{
            printf("%d('%c')\n\r",c,c);
        }
    }

    return 0;
}
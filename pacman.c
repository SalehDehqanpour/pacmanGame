#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <time.h>


#define HEIGHT 30  //Number of Rows of maze in terms of characters
#define WIDTH 60	//Number of Collumns of mazein terms of characters

#define BOUNDARY 2 //BOUNDARY=2 means monsters will chase pacman if they place in a 2*2 square around pacman.(bonus section)

#define ACCIDENT 1  	// means monster should change direction due to a barrier in front of it
#define NO_ACCIDENT 0

#define TOTAL_DOTS 611  // Maximum score that pacman could gain

// used for defining color schemes:
#define BLANK 0
#define BAR 1
#define PACMAN1 2
#define PACMAN2 3
#define MONSTER1 4
#define MONSTER2 5
#define FLASH 6

typedef struct DATA Data;
typedef struct PACMAN Pacman;
typedef struct MONSTER Monster;
typedef struct MAZE Maze;


void* keyboard (void*);
void* game (void*);
void* show (void*);
void init (void);
short updateMonster(Monster*,int*,int*);
void initMaze(void);
void eating_a_monster(int,int); // pacman eats a monster in powerful mode. In the case, this function performs necessary tasks.


struct PACMAN {
	int x,y;
	unsigned char power; // 1: pacman has eaten a power dot(flashing dot); 0 :normal mode i.e. no super power
	unsigned int powerTimer; 
	unsigned char heart; // between 0 and 3; 0 means losing
	unsigned int score; // number of dots eaten
	int startx,starty;
	unsigned char dir;//direction
} pacman;

struct MONSTER {
	int x,y;
	int startx,starty;
	unsigned char underlying; // value that current monster cell has before monster enters it	
	unsigned char dir;//direction
	unsigned char defeated;//If a monster is eaten, it becomes impregnable untill pacman eat another flashing dot.
	// defeated values: 1: monster has been eaten, 0: otherwise
	
} M1,M2,M3;

struct MAZE {
	unsigned char matrix[WIDTH][HEIGHT] ;
	
	
} maze;

struct DATA {
	struct PACMAN* ppacman; // pointer to pacman
	struct MONSTER* pmonster[3];
	struct MAZE* pmaze;
	
}data;
int main (int argc, char* argv[]){
	
	srand(time(NULL));
	
	
	initscr();
	raw(); // Disabling keyboard buffer
	noecho(); //turn off displaying characters user type
	keypad(stdscr,1); //get special keys from user
	curs_set(0); //disable blinking cursor
	start_color(); // enable color
	
	init_pair(BLANK , COLOR_WHITE , COLOR_BLACK);
	init_pair(BAR , COLOR_BLACK ,COLOR_WHITE);//barrier
	init_pair(PACMAN1 , COLOR_WHITE , COLOR_BLUE);//normal pacman
	init_pair(PACMAN2 , COLOR_WHITE , COLOR_GREEN);//powerful pacman
	init_pair(MONSTER1 , COLOR_WHITE , COLOR_RED);//normal monster
	init_pair(MONSTER2 , COLOR_CYAN , COLOR_RED);//weak monster
	init_pair(FLASH, COLOR_GREEN , COLOR_BLACK);//power dots

	
	
	
	init();
	pthread_t handle0, handle1, handle2;
 

	pthread_create( &handle0, NULL, game , (void*) NULL/*(&data)*/ ); 	
	pthread_create( &handle1, NULL, show , (void*) NULL );
	pthread_create( &handle2, NULL, keyboard, (void*) NULL);
	pthread_join( handle0, NULL);
	pthread_join( handle1, NULL); 
	pthread_join( handle2, NULL); 


	// End of the game. Displaying Score and some messages
	short rowsOfstdscr,colsOfstdscr;
	clear();//clears screen
	getmaxyx(stdscr,rowsOfstdscr,colsOfstdscr);		// get the number of rows and columns 
	if(pacman.heart) // means the game is not ended due to lack of hearts. Thus player has won.
		mvprintw(rowsOfstdscr/2,(colsOfstdscr-36)/2,"Congratulations!You has won the game");// print at center of screen;36 is length of string argument
	else 
		mvprintw(rowsOfstdscr/2,(colsOfstdscr-10)/2,"Game Over!");// print at center of screen
	  
	 mvprintw(rowsOfstdscr/2+2,(colsOfstdscr-21)/2,"Your Final Score: %d",pacman.score);
	 mvprintw(rowsOfstdscr/2+2,(colsOfstdscr-21)/2,"Press any key to exit");
	 refresh();
	
	
		getch();

	endwin();
	
	return 0;
	
	
}




void init (void){

	
	data.ppacman=&pacman;
	data.pmaze=&maze;
	*(data.pmonster)=&M1;
	*((data.pmonster) +1 )= &M2;
	*((data.pmonster) +2)= &M3;
	
	pacman.power=0;
	pacman.powerTimer=0;
	pacman.heart=3;
	pacman.score=0;
	pacman.startx=3;
	pacman.starty=2 ;
	pacman.x=pacman.startx;
	pacman.y=pacman.starty;
	pacman.dir=0;

	
	M1.startx=51;
	M1.starty=6;
	M1.x=M1.startx;
	M1.y=M1.starty;	
	M1.underlying=1;
	M1.dir=1;
	M1.defeated=0;
	
	M2.startx=16;
	M2.starty=12;
	M2.x=M2.startx;
	M2.y=M2.starty;	
	M2.underlying=1;
	M2.dir=2;
	M2.defeated=0;
	
	M3.startx=47;
	M3.starty=28;
	M3.x=M3.startx;
	M3.y=M3.starty;	
	M3.underlying=1;
	M3.dir=3;
	M3.defeated=0;
	

	initMaze();

};









void* game (void* d){
	int newx,newy;
	unsigned char p1=0,m1=0,m2=0;// a counter to update pacman and normal & weak monster for adjusting speeds of elements
	 while (pacman.heart!=0 && pacman.score<TOTAL_DOTS){
		 //Updating monsters
		 if ((!m1 && !pacman.power)||(!m2 && pacman.power)) {
			 while(updateMonster(&M1,&newx,&newy)){M1.dir=1+rand()%4;}	 
			 while(updateMonster(&M2,&newx,&newy)){M2.dir=1+rand()%4;}
			 while(updateMonster(&M3,&newx,&newy)){M3.dir=1+rand()%4;}
		 
		 
		 
		 }
		 
		
		 
		 
		 
		 if (!p1) {
		 switch (pacman.dir){ // figure out next cell from direction and current position
			case 1: newx=pacman.x;newy=pacman.y+1;
					break;
			case 2:newx=pacman.x+1;newy=pacman.y;
					break;
			case 3:newx=pacman.x;newy=pacman.y-1;
					break;
			case 4:newx=pacman.x-1;newy=pacman.y;
					break;
		
		 }
			
			switch (maze.matrix[newx][newy]){
				case 0: 	// next cell is a barrier
							break;
				case 1: 	// next cell is a dot
							maze.matrix[pacman.x][pacman.y]=2;
							pacman.x=newx;		
							pacman.y=newy;
							pacman.score++;
							maze.matrix[pacman.x][pacman.y]=pacman.power?4:3;
							break;
				case 2: 	// next cell is a blank cell 
							maze.matrix[pacman.x][pacman.y]=2;
							pacman.x=newx;		
							pacman.y=newy;
							maze.matrix[pacman.x][pacman.y]=pacman.power?4:3;
							break;
				case 5: 	// next cell is a monster (normal mode)
							maze.matrix[pacman.x][pacman.y]=2;
							pacman.heart--;
							newx=pacman.startx;
							newy=pacman.starty;
							pacman.x=pacman.startx;		
							pacman.y=pacman.starty;								
							maze.matrix[pacman.x][pacman.y]=3;	
							pacman.dir=0;	
							
							break;
				case 6: 	// next cell is a monster (weak mode)
							eating_a_monster(newx,newy);	
							break;
				case 7: 	// next cell is a flashing dot
							maze.matrix[pacman.x][pacman.y]=2;
							pacman.power=1;
							pacman.powerTimer=33; // 5 seconds (33 * 150 ms = 5 s)
							pacman.x=newx;		
							pacman.y=newy;
							maze.matrix[pacman.x][pacman.y]=4;
							break;
				

			}
		 

		 }
		 
		 
		 
	
	napms(150);
	p1=(p1+1)%3;
	m1=(m1+1)%4;
	m2=(m2+1)%6;// adjusting speeds of elements
	
	if (pacman.powerTimer) pacman.powerTimer--; 	
		else {
			pacman.power=0;
			M1.defeated=0;
			M2.defeated=0;
			M3.defeated=0;
			}  // establishing a timer for powerful pacman. It is set to 5 seconds.
	}

}

void* show (void* d){
	short i,j;
	while (pacman.heart!=0 && pacman.score<TOTAL_DOTS) {

	
	for (i=0;i<WIDTH; i++){
		for (j=0;j<HEIGHT; j++){
			switch(maze.matrix[i][j]){
	case 0: mvaddch(j,i,' ' | COLOR_PAIR(BAR));break;
	case 1:	mvaddch(j,i,'.' | COLOR_PAIR(BLANK));break;
	case 2: mvaddch(j,i,' ' | COLOR_PAIR(BLANK));break;
	case 3: mvaddch(j,i,'P' | COLOR_PAIR(PACMAN1));break;
	case 4: mvaddch(j,i,'P' | COLOR_PAIR(PACMAN2));break;
	case 5: mvaddch(j,i,'X' | COLOR_PAIR(MONSTER1));break;	
	case 6: mvaddch(j,i,'x' | COLOR_PAIR(MONSTER2) | A_BLINK);break;	
	case 7: mvaddch(j,i,'.' | COLOR_PAIR(FLASH) | A_BLINK);break;	
	
		
		}}}
		mvprintw(HEIGHT+1,0,"Your Score: %d \tYour Heart: %d",pacman.score,pacman.heart);
		mvprintw(HEIGHT+2,0,"Press q to exit game");
		
	refresh();napms(150);
	}
	
	
	
	
	
}

void* keyboard (void* d){
	int ch,new_x,new_y,defaultx,defaulty;
	short rowsOfstdscr,colsOfstdscr;
	unsigned char new_dir;
		while (pacman.heart!=0 && pacman.score<TOTAL_DOTS) {
			
		ch = getch();flushinp();
		switch (ch){
			case KEY_LEFT:	new_dir=4;break;
			case KEY_RIGHT:	new_dir=2;break;
			case KEY_UP:	new_dir=3;break;
			case KEY_DOWN:	new_dir=1;break;
			case 'q' :	//for exiting the game and go to terminal emulator
					clear(); 
					pacman.heart=0;
					getmaxyx(stdscr,rowsOfstdscr,colsOfstdscr);
					mvprintw(rowsOfstdscr/2+2,(colsOfstdscr-31)/2,"Good Bye! Press any key to exit");
					refresh();
					getch();
					endwin();
					exit(0);
					break;
		}
		switch (new_dir){
			 case 1:new_x=pacman.x;
					new_y=pacman.y+1;
					break;
			 case 2:new_x=pacman.x+1;
					new_y=pacman.y;
					break;
			 case 3:new_x=pacman.x;
					new_y=pacman.y-1;
					break;
			 case 4:new_x=pacman.x-1;
					new_y=pacman.y;
					break;
		
		 }
		 
		switch (pacman.dir){
			 case 1:defaultx=pacman.x;
					defaulty=pacman.y+1;
					break;
			 case 2:defaultx=pacman.x+1;
					defaulty=pacman.y;
					break;
			 case 3:defaultx=pacman.x;
					defaulty=pacman.y-1;
					break;
			 case 4:defaultx=pacman.x-1;
					defaulty=pacman.y;
					break;
			default : pacman.dir=new_dir; 
					break;
		 }
		if ((maze.matrix[new_x][new_y]!=0)) 
			pacman.dir=new_dir; // pacman changes direction if and only if the new path that user desires is a open path(not barrier).
								// Otherwise pacman keeps on going on its current direction
	napms(300);
		}
}




short updateMonster(Monster* m,int* pacmanNextx,int* pacmanNexty){
	int newx,newy;
	Monster* aux; // used for handling collision of two monsters
	if (abs(pacman.x - m->x)<=BOUNDARY && abs(pacman.y - m->y)<=BOUNDARY) {// code for bonus mark
		
		//Checking which one of 4 cells around monster is closer to pacman and also is not a barrier
		if (m->x - pacman.x > 0 &&   maze.matrix[m->x -1][m->y]!=0)	{m->dir=pacman.power?2:4;} // tracking or escaping from pacman dependen on pacman powerfulness
		else if (m->y - pacman.y > 0 &&   maze.matrix[m->x][m->y +1]!=0)	{m->dir=pacman.power?1:3;}
		else if (m->x - pacman.x < 0 &&   maze.matrix[m->x+1][m->y]!=0)	{m->dir=pacman.power?4:2;}
		else if (m->y - pacman.y < 0 &&   maze.matrix[m->x][m->y -1]!=0)	{m->dir=pacman.power?3:1;} 
		
		}
	
	
	switch (m->dir){
		 case 1: newx=m->x;
				newy=m->y+1;
				break;
		 case 2:newx=m->x+1;
				newy=m->y;
				break;
		 case 3:newx=m->x;
				newy=m->y-1;
				break;
		 case 4:newx=m->x-1;
				newy=m->y;
				break;
	}
		
		switch (maze.matrix[newx][newy]){
				case 0: 	// next cell is a barrier
							return ACCIDENT;
							break;
				case 1: 	// next cell is a dot
				case 2: 	// next cell is a blank cell
				case 7: 	// next cell is a flashing dot
							// It does not make difference for a monster whether next cell is a dot or blank or flashing dot
							maze.matrix[m->x][m->y]=m->underlying;
							m->x=newx;
							m->y=newy;
							m->underlying=maze.matrix[m->x][m->y];
							if (m->underlying==3 || m->underlying==4) m->underlying=2;
							maze.matrix[m->x][m->y]=((!m->defeated) && pacman.power)?6:5;
							return NO_ACCIDENT;
							break;
				
				case 3: 	// This case is merely a patch for a minor bug when pacman is stopped beside a barrier...
							// and monster has arrived to it.
							if (maze.matrix[(*pacmanNextx)][(*pacmanNexty)]==0) {
							maze.matrix[m->x][m->y]=m->underlying;
							m->x=newx;
							m->y=newy;
							m->underlying=2; // the cell that pacman was there, will be a blank cell
							maze.matrix[m->x][m->y]=5;
							pacman.heart--;
							*pacmanNextx=pacman.startx;
							*pacmanNexty=pacman.starty;
							pacman.x=pacman.startx;		
							pacman.y=pacman.starty;								
							maze.matrix[pacman.x][pacman.y]=3;	
							pacman.dir=0;
					
							break;
					
				}
				
				
				
				case 5: 	// next cell is a monster (normal mode)
				case 6: 	// next cell is a monster (weak mode)
							//crossing two monsters. Several steps are needed as follows:
							
							//figuring out adjacent monster
							if (M1.x==newx && M1.y==newy) aux=&M1;
							else if (M2.x==newx && M2.y==newy) aux=&M2;
							else if (M3.x==newx && M3.y==newy) aux=&M3;
							
							// Substituition operation:
							maze.matrix[m->x][m->y]=m->underlying;
							m->x=newx;
							m->y=newy;
							maze.matrix[m->x][m->y]=((!m->defeated) && pacman.power)?6:5;
							m->underlying=aux->underlying;							
							if (m->underlying==3 || m->underlying==4) m->underlying=2;
							aux->underlying=maze.matrix[m->x][m->y];
							if (aux->underlying==3 || aux->underlying==4) aux->underlying=2;
							return NO_ACCIDENT;
							break;
				default : 	// If next cell is pacman, the problem will be handled by code of pacman section
							return NO_ACCIDENT; 
				
				
				
				
			}
		
	
}


void eating_a_monster(int newx,int newy){
	
maze.matrix[pacman.x][pacman.y]=2;
pacman.x=newx;		
pacman.y=newy;

maze.matrix[pacman.x][pacman.y]=4;
	
	Monster* defeatedMonster;
	if (M1.x==pacman.x && M1.y==pacman.y) defeatedMonster=&M1;
	else if (M2.x==pacman.x && M2.y==pacman.y) defeatedMonster=&M2;
	else if (M3.x==pacman.x && M3.y==pacman.y) defeatedMonster=&M3;
	
	if(defeatedMonster->underlying==1) pacman.score++;
	
	defeatedMonster->x=defeatedMonster->startx;
	defeatedMonster->y=defeatedMonster->starty;
	defeatedMonster->defeated=1;
	maze.matrix[defeatedMonster->x][defeatedMonster->y]=5;
}



void initMaze(void){
	
	//initializing maze matrix so we have a complete maze having barriers, dots, flashing dots, ...
	short i,j;
	for (i=0;i<WIDTH; i++)
		for (j=0;j<HEIGHT; j++)
	maze.matrix[i][j]=0; //first set all cells as barrier (most of cells are of this type) then set dots, etc.
	 
	 // writing the maze is fully automated! I has adapted a maze matrix from internet and write a matlab script to find 
	 // index of dots.(by "find" MATLAB function).Then putting brackets around them. My maze file is uploaded in : http://ee.sharif.ir/~dehqanpour_ms/maze.txt
	 // Thanks to GNU text editor Notepad++ for providing column operations.
maze.matrix[ 3][ 2]=1;
maze.matrix[ 3][ 3]=1;
maze.matrix[ 3][ 4]=1;
maze.matrix[ 3][ 5]=1;
maze.matrix[ 3][ 6]=1;
maze.matrix[ 3][ 7]=1;
maze.matrix[ 3][ 8]=1;
maze.matrix[ 3][ 9]=1;
maze.matrix[ 3][14]=1;
maze.matrix[ 3][19]=1;
maze.matrix[ 3][20]=1;
maze.matrix[ 3][21]=1;
maze.matrix[ 3][22]=1;
maze.matrix[ 3][25]=1;
maze.matrix[ 3][26]=1;
maze.matrix[ 3][27]=1;
maze.matrix[ 3][28]=1;
maze.matrix[ 4][ 2]=1;
maze.matrix[ 4][ 6]=1;
maze.matrix[ 4][ 9]=1;
maze.matrix[ 4][14]=1;
maze.matrix[ 4][19]=1;
maze.matrix[ 4][22]=1;
maze.matrix[ 4][25]=1;
maze.matrix[ 4][28]=1;
maze.matrix[ 5][ 2]=1;
maze.matrix[ 5][ 6]=1;
maze.matrix[ 5][ 9]=1;
maze.matrix[ 5][10]=1;
maze.matrix[ 5][11]=1;
maze.matrix[ 5][12]=1;
maze.matrix[ 5][13]=1;
maze.matrix[ 5][14]=1;
maze.matrix[ 5][19]=1;
maze.matrix[ 5][22]=1;
maze.matrix[ 5][23]=1;
maze.matrix[ 5][24]=1;
maze.matrix[ 5][25]=1;
maze.matrix[ 5][28]=1;
maze.matrix[ 6][ 2]=1;
maze.matrix[ 6][ 6]=1;
maze.matrix[ 6][ 9]=1;
maze.matrix[ 6][14]=1;
maze.matrix[ 6][19]=1;
maze.matrix[ 6][25]=1;
maze.matrix[ 6][26]=1;
maze.matrix[ 6][27]=1;
maze.matrix[ 6][28]=1;
maze.matrix[ 7][ 2]=1;
maze.matrix[ 7][ 6]=1;
maze.matrix[ 7][ 9]=1;
maze.matrix[ 7][14]=1;
maze.matrix[ 7][19]=1;
maze.matrix[ 7][25]=1;
maze.matrix[ 7][28]=1;
maze.matrix[ 8][ 2]=1;
maze.matrix[ 8][ 3]=1;
maze.matrix[ 8][ 4]=1;
maze.matrix[ 8][ 5]=1;
maze.matrix[ 8][ 6]=1;
maze.matrix[ 8][ 7]=1;
maze.matrix[ 8][ 8]=1;
maze.matrix[ 8][ 9]=1;
maze.matrix[ 8][10]=1;
maze.matrix[ 8][11]=1;
maze.matrix[ 8][12]=1;
maze.matrix[ 8][13]=1;
maze.matrix[ 8][14]=1;
maze.matrix[ 8][15]=1;
maze.matrix[ 8][16]=1;
maze.matrix[ 8][17]=1;
maze.matrix[ 8][18]=1;
maze.matrix[ 8][19]=1;
maze.matrix[ 8][20]=1;
maze.matrix[ 8][21]=1;
maze.matrix[ 8][22]=1;
maze.matrix[ 8][23]=1;
maze.matrix[ 8][24]=1;
maze.matrix[ 8][25]=1;
maze.matrix[ 8][28]=1;
maze.matrix[ 9][ 2]=1;
maze.matrix[ 9][ 6]=1;
maze.matrix[ 9][14]=1;
maze.matrix[ 9][19]=1;
maze.matrix[ 9][22]=1;
maze.matrix[ 9][28]=1;
maze.matrix[10][ 2]=1;
maze.matrix[10][ 6]=1;
maze.matrix[10][14]=1;
maze.matrix[10][19]=1;
maze.matrix[10][22]=1;
maze.matrix[10][28]=1;
maze.matrix[11][ 2]=1;
maze.matrix[11][ 6]=1;
maze.matrix[11][ 7]=1;
maze.matrix[11][ 8]=1;
maze.matrix[11][ 9]=1;
maze.matrix[11][12]=1;
maze.matrix[11][13]=1;
maze.matrix[11][14]=1;
maze.matrix[11][15]=1;
maze.matrix[11][16]=1;
maze.matrix[11][17]=1;
maze.matrix[11][18]=1;
maze.matrix[11][19]=1;
maze.matrix[11][22]=1;
maze.matrix[11][23]=1;
maze.matrix[11][24]=1;
maze.matrix[11][25]=1;
maze.matrix[11][28]=1;
maze.matrix[12][ 2]=1;
maze.matrix[12][ 6]=1;
maze.matrix[12][ 9]=1;
maze.matrix[12][12]=1;
maze.matrix[12][16]=1;
maze.matrix[12][19]=1;
maze.matrix[12][22]=1;
maze.matrix[12][25]=1;
maze.matrix[12][28]=1;
maze.matrix[13][ 2]=1;
maze.matrix[13][ 6]=1;
maze.matrix[13][ 9]=1;
maze.matrix[13][12]=1;
maze.matrix[13][16]=1;
maze.matrix[13][19]=1;
maze.matrix[13][22]=1;
maze.matrix[13][25]=1;
maze.matrix[13][28]=1;
maze.matrix[14][ 2]=1;
maze.matrix[14][ 3]=1;
maze.matrix[14][ 4]=1;
maze.matrix[14][ 5]=1;
maze.matrix[14][ 6]=1;
maze.matrix[14][ 9]=1;
maze.matrix[14][10]=1;
maze.matrix[14][11]=1;
maze.matrix[14][12]=1;
maze.matrix[14][16]=1;
maze.matrix[14][19]=1;
maze.matrix[14][20]=1;
maze.matrix[14][21]=1;
maze.matrix[14][22]=1;
maze.matrix[14][25]=1;
maze.matrix[14][26]=1;
maze.matrix[14][27]=1;
maze.matrix[14][28]=1;
maze.matrix[15][ 6]=1;
maze.matrix[15][12]=1;
maze.matrix[15][16]=1;
maze.matrix[15][22]=1;
maze.matrix[15][28]=1;
maze.matrix[16][ 6]=1;
maze.matrix[16][12]=1;
maze.matrix[16][16]=1;
maze.matrix[16][22]=1;
maze.matrix[16][28]=1;
maze.matrix[17][ 2]=1;
maze.matrix[17][ 3]=1;
maze.matrix[17][ 4]=1;
maze.matrix[17][ 5]=1;
maze.matrix[17][ 6]=1;
maze.matrix[17][ 9]=1;
maze.matrix[17][10]=1;
maze.matrix[17][11]=1;
maze.matrix[17][12]=1;
maze.matrix[17][16]=1;
maze.matrix[17][19]=1;
maze.matrix[17][20]=1;
maze.matrix[17][21]=1;
maze.matrix[17][22]=1;
maze.matrix[17][25]=1;
maze.matrix[17][26]=1;
maze.matrix[17][27]=1;
maze.matrix[17][28]=1;
maze.matrix[18][ 2]=1;
maze.matrix[18][ 6]=1;
maze.matrix[18][ 9]=1;
maze.matrix[18][12]=1;
maze.matrix[18][16]=1;
maze.matrix[18][19]=1;
maze.matrix[18][22]=1;
maze.matrix[18][25]=1;
maze.matrix[18][28]=1;
maze.matrix[19][ 2]=1;
maze.matrix[19][ 6]=1;
maze.matrix[19][ 9]=1;
maze.matrix[19][12]=1;
maze.matrix[19][16]=1;
maze.matrix[19][19]=1;
maze.matrix[19][22]=1;
maze.matrix[19][25]=1;
maze.matrix[19][28]=1;
maze.matrix[20][ 2]=1;
maze.matrix[20][ 6]=1;
maze.matrix[20][ 7]=1;
maze.matrix[20][ 8]=1;
maze.matrix[20][ 9]=1;
maze.matrix[20][12]=1;
maze.matrix[20][13]=1;
maze.matrix[20][14]=1;
maze.matrix[20][15]=1;
maze.matrix[20][16]=1;
maze.matrix[20][17]=1;
maze.matrix[20][18]=1;
maze.matrix[20][19]=1;
maze.matrix[20][22]=1;
maze.matrix[20][23]=1;
maze.matrix[20][24]=1;
maze.matrix[20][25]=1;
maze.matrix[20][28]=1;
maze.matrix[21][ 2]=1;
maze.matrix[21][ 6]=1;
maze.matrix[21][14]=1;
maze.matrix[21][19]=1;
maze.matrix[21][22]=1;
maze.matrix[21][28]=1;
maze.matrix[22][ 2]=1;
maze.matrix[22][ 6]=1;
maze.matrix[22][14]=1;
maze.matrix[22][19]=1;
maze.matrix[22][22]=1;
maze.matrix[22][28]=1;
maze.matrix[23][ 2]=1;
maze.matrix[23][ 3]=1;
maze.matrix[23][ 4]=1;
maze.matrix[23][ 5]=1;
maze.matrix[23][ 6]=1;
maze.matrix[23][ 7]=1;
maze.matrix[23][ 8]=1;
maze.matrix[23][ 9]=1;
maze.matrix[23][10]=1;
maze.matrix[23][11]=1;
maze.matrix[23][12]=1;
maze.matrix[23][13]=1;
maze.matrix[23][14]=1;
maze.matrix[23][15]=1;
maze.matrix[23][16]=1;
maze.matrix[23][17]=1;
maze.matrix[23][18]=1;
maze.matrix[23][19]=1;
maze.matrix[23][20]=1;
maze.matrix[23][21]=1;
maze.matrix[23][22]=1;
maze.matrix[23][23]=1;
maze.matrix[23][24]=1;
maze.matrix[23][25]=1;
maze.matrix[23][28]=1;
maze.matrix[24][ 2]=1;
maze.matrix[24][ 6]=1;
maze.matrix[24][ 9]=1;
maze.matrix[24][14]=1;
maze.matrix[24][19]=1;
maze.matrix[24][25]=1;
maze.matrix[24][28]=1;
maze.matrix[25][ 2]=1;
maze.matrix[25][ 6]=1;
maze.matrix[25][ 9]=1;
maze.matrix[25][14]=1;
maze.matrix[25][19]=1;
maze.matrix[25][25]=1;
maze.matrix[25][26]=1;
maze.matrix[25][27]=1;
maze.matrix[25][28]=1;
maze.matrix[26][ 2]=1;
maze.matrix[26][ 6]=1;
maze.matrix[26][ 9]=1;
maze.matrix[26][10]=1;
maze.matrix[26][11]=1;
maze.matrix[26][12]=1;
maze.matrix[26][13]=1;
maze.matrix[26][14]=1;
maze.matrix[26][19]=1;
maze.matrix[26][22]=1;
maze.matrix[26][23]=1;
maze.matrix[26][24]=1;
maze.matrix[26][25]=1;
maze.matrix[26][28]=1;
maze.matrix[27][ 2]=1;
maze.matrix[27][ 6]=1;
maze.matrix[27][ 9]=1;
maze.matrix[27][14]=1;
maze.matrix[27][19]=1;
maze.matrix[27][22]=1;
maze.matrix[27][25]=1;
maze.matrix[27][28]=1;
maze.matrix[28][ 2]=1;
maze.matrix[28][ 3]=1;
maze.matrix[28][ 4]=1;
maze.matrix[28][ 5]=1;
maze.matrix[28][ 6]=1;
maze.matrix[28][ 7]=1;
maze.matrix[28][ 8]=1;
maze.matrix[28][ 9]=1;
maze.matrix[28][14]=1;
maze.matrix[28][19]=1;
maze.matrix[28][20]=1;
maze.matrix[28][21]=1;
maze.matrix[28][22]=1;
maze.matrix[28][25]=1;
maze.matrix[28][26]=1;
maze.matrix[28][27]=1;
maze.matrix[28][28]=1;
maze.matrix[29][ 6]=1;
maze.matrix[29][ 9]=1;
maze.matrix[29][14]=1;
maze.matrix[29][19]=1;
maze.matrix[29][22]=1;
maze.matrix[29][25]=1;
maze.matrix[30][ 6]=1;
maze.matrix[30][ 9]=1;
maze.matrix[30][14]=1;
maze.matrix[30][19]=1;
maze.matrix[30][22]=1;
maze.matrix[30][25]=1;
maze.matrix[31][ 2]=1;
maze.matrix[31][ 3]=1;
maze.matrix[31][ 4]=1;
maze.matrix[31][ 5]=1;
maze.matrix[31][ 6]=1;
maze.matrix[31][ 7]=1;
maze.matrix[31][ 8]=1;
maze.matrix[31][ 9]=1;
maze.matrix[31][14]=1;
maze.matrix[31][19]=1;
maze.matrix[31][20]=1;
maze.matrix[31][21]=1;
maze.matrix[31][22]=1;
maze.matrix[31][25]=1;
maze.matrix[31][26]=1;
maze.matrix[31][27]=1;
maze.matrix[31][28]=1;
maze.matrix[32][ 2]=1;
maze.matrix[32][ 6]=1;
maze.matrix[32][ 9]=1;
maze.matrix[32][14]=1;
maze.matrix[32][19]=1;
maze.matrix[32][22]=1;
maze.matrix[32][25]=1;
maze.matrix[32][28]=1;
maze.matrix[33][ 2]=1;
maze.matrix[33][ 6]=1;
maze.matrix[33][ 9]=1;
maze.matrix[33][10]=1;
maze.matrix[33][11]=1;
maze.matrix[33][12]=1;
maze.matrix[33][13]=1;
maze.matrix[33][14]=1;
maze.matrix[33][19]=1;
maze.matrix[33][22]=1;
maze.matrix[33][23]=1;
maze.matrix[33][24]=1;
maze.matrix[33][25]=1;
maze.matrix[33][28]=1;
maze.matrix[34][ 2]=1;
maze.matrix[34][ 6]=1;
maze.matrix[34][ 9]=1;
maze.matrix[34][14]=1;
maze.matrix[34][19]=1;
maze.matrix[34][25]=1;
maze.matrix[34][26]=1;
maze.matrix[34][27]=1;
maze.matrix[34][28]=1;
maze.matrix[35][ 2]=1;
maze.matrix[35][ 6]=1;
maze.matrix[35][ 9]=1;
maze.matrix[35][14]=1;
maze.matrix[35][19]=1;
maze.matrix[35][25]=1;
maze.matrix[35][28]=1;
maze.matrix[36][ 2]=1;
maze.matrix[36][ 3]=1;
maze.matrix[36][ 4]=1;
maze.matrix[36][ 5]=1;
maze.matrix[36][ 6]=1;
maze.matrix[36][ 7]=1;
maze.matrix[36][ 8]=1;
maze.matrix[36][ 9]=1;
maze.matrix[36][10]=1;
maze.matrix[36][11]=1;
maze.matrix[36][12]=1;
maze.matrix[36][13]=1;
maze.matrix[36][14]=1;
maze.matrix[36][15]=1;
maze.matrix[36][16]=1;
maze.matrix[36][17]=1;
maze.matrix[36][18]=1;
maze.matrix[36][19]=1;
maze.matrix[36][20]=1;
maze.matrix[36][21]=1;
maze.matrix[36][22]=1;
maze.matrix[36][23]=1;
maze.matrix[36][24]=1;
maze.matrix[36][25]=1;
maze.matrix[36][28]=1;
maze.matrix[37][ 2]=1;
maze.matrix[37][ 6]=1;
maze.matrix[37][14]=1;
maze.matrix[37][19]=1;
maze.matrix[37][22]=1;
maze.matrix[37][28]=1;
maze.matrix[38][ 2]=1;
maze.matrix[38][ 6]=1;
maze.matrix[38][14]=1;
maze.matrix[38][19]=1;
maze.matrix[38][22]=1;
maze.matrix[38][28]=1;
maze.matrix[39][ 2]=1;
maze.matrix[39][ 6]=1;
maze.matrix[39][ 7]=1;
maze.matrix[39][ 8]=1;
maze.matrix[39][ 9]=1;
maze.matrix[39][12]=1;
maze.matrix[39][13]=1;
maze.matrix[39][14]=1;
maze.matrix[39][15]=1;
maze.matrix[39][16]=1;
maze.matrix[39][17]=1;
maze.matrix[39][18]=1;
maze.matrix[39][19]=1;
maze.matrix[39][22]=1;
maze.matrix[39][23]=1;
maze.matrix[39][24]=1;
maze.matrix[39][25]=1;
maze.matrix[39][28]=1;
maze.matrix[40][ 2]=1;
maze.matrix[40][ 6]=1;
maze.matrix[40][ 9]=1;
maze.matrix[40][12]=1;
maze.matrix[40][16]=1;
maze.matrix[40][19]=1;
maze.matrix[40][22]=1;
maze.matrix[40][25]=1;
maze.matrix[40][28]=1;
maze.matrix[41][ 2]=1;
maze.matrix[41][ 6]=1;
maze.matrix[41][ 9]=1;
maze.matrix[41][12]=1;
maze.matrix[41][16]=1;
maze.matrix[41][19]=1;
maze.matrix[41][22]=1;
maze.matrix[41][25]=1;
maze.matrix[41][28]=1;
maze.matrix[42][ 2]=1;
maze.matrix[42][ 3]=1;
maze.matrix[42][ 4]=1;
maze.matrix[42][ 5]=1;
maze.matrix[42][ 6]=1;
maze.matrix[42][ 9]=1;
maze.matrix[42][10]=1;
maze.matrix[42][11]=1;
maze.matrix[42][12]=1;
maze.matrix[42][16]=1;
maze.matrix[42][19]=1;
maze.matrix[42][20]=1;
maze.matrix[42][21]=1;
maze.matrix[42][22]=1;
maze.matrix[42][25]=1;
maze.matrix[42][26]=1;
maze.matrix[42][27]=1;
maze.matrix[42][28]=1;
maze.matrix[43][ 6]=1;
maze.matrix[43][12]=1;
maze.matrix[43][16]=1;
maze.matrix[43][22]=1;
maze.matrix[43][28]=1;
maze.matrix[44][ 6]=1;
maze.matrix[44][12]=1;
maze.matrix[44][16]=1;
maze.matrix[44][22]=1;
maze.matrix[44][28]=1;
maze.matrix[45][ 2]=1;
maze.matrix[45][ 3]=1;
maze.matrix[45][ 4]=1;
maze.matrix[45][ 5]=1;
maze.matrix[45][ 6]=1;
maze.matrix[45][ 9]=1;
maze.matrix[45][10]=1;
maze.matrix[45][11]=1;
maze.matrix[45][12]=1;
maze.matrix[45][16]=1;
maze.matrix[45][19]=1;
maze.matrix[45][20]=1;
maze.matrix[45][21]=1;
maze.matrix[45][22]=1;
maze.matrix[45][25]=1;
maze.matrix[45][26]=1;
maze.matrix[45][27]=1;
maze.matrix[45][28]=1;
maze.matrix[46][ 2]=1;
maze.matrix[46][ 6]=1;
maze.matrix[46][ 9]=1;
maze.matrix[46][12]=1;
maze.matrix[46][16]=1;
maze.matrix[46][19]=1;
maze.matrix[46][22]=1;
maze.matrix[46][25]=1;
maze.matrix[46][28]=1;
maze.matrix[47][ 2]=1;
maze.matrix[47][ 6]=1;
maze.matrix[47][ 9]=1;
maze.matrix[47][12]=1;
maze.matrix[47][16]=1;
maze.matrix[47][19]=1;
maze.matrix[47][22]=1;
maze.matrix[47][25]=1;
maze.matrix[47][28]=1;
maze.matrix[48][ 2]=1;
maze.matrix[48][ 6]=1;
maze.matrix[48][ 7]=1;
maze.matrix[48][ 8]=1;
maze.matrix[48][ 9]=1;
maze.matrix[48][12]=1;
maze.matrix[48][13]=1;
maze.matrix[48][14]=1;
maze.matrix[48][15]=1;
maze.matrix[48][16]=1;
maze.matrix[48][17]=1;
maze.matrix[48][18]=1;
maze.matrix[48][19]=1;
maze.matrix[48][22]=1;
maze.matrix[48][23]=1;
maze.matrix[48][24]=1;
maze.matrix[48][25]=1;
maze.matrix[48][28]=1;
maze.matrix[49][ 2]=1;
maze.matrix[49][ 6]=1;
maze.matrix[49][14]=1;
maze.matrix[49][19]=1;
maze.matrix[49][22]=1;
maze.matrix[49][28]=1;
maze.matrix[50][ 2]=1;
maze.matrix[50][ 6]=1;
maze.matrix[50][14]=1;
maze.matrix[50][19]=1;
maze.matrix[50][22]=1;
maze.matrix[50][28]=1;
maze.matrix[51][ 2]=1;
maze.matrix[51][ 3]=1;
maze.matrix[51][ 4]=1;
maze.matrix[51][ 5]=1;
maze.matrix[51][ 6]=1;
maze.matrix[51][ 7]=1;
maze.matrix[51][ 8]=1;
maze.matrix[51][ 9]=1;
maze.matrix[51][10]=1;
maze.matrix[51][11]=1;
maze.matrix[51][12]=1;
maze.matrix[51][13]=1;
maze.matrix[51][14]=1;
maze.matrix[51][15]=1;
maze.matrix[51][16]=1;
maze.matrix[51][17]=1;
maze.matrix[51][18]=1;
maze.matrix[51][19]=1;
maze.matrix[51][20]=1;
maze.matrix[51][21]=1;
maze.matrix[51][22]=1;
maze.matrix[51][23]=1;
maze.matrix[51][24]=1;
maze.matrix[51][25]=1;
maze.matrix[51][28]=1;
maze.matrix[52][ 2]=1;
maze.matrix[52][ 6]=1;
maze.matrix[52][ 9]=1;
maze.matrix[52][14]=1;
maze.matrix[52][19]=1;
maze.matrix[52][25]=1;
maze.matrix[52][28]=1;
maze.matrix[53][ 2]=1;
maze.matrix[53][ 6]=1;
maze.matrix[53][ 9]=1;
maze.matrix[53][14]=1;
maze.matrix[53][19]=1;
maze.matrix[53][25]=1;
maze.matrix[53][26]=1;
maze.matrix[53][27]=1;
maze.matrix[53][28]=1;
maze.matrix[54][ 2]=1;
maze.matrix[54][ 6]=1;
maze.matrix[54][ 9]=1;
maze.matrix[54][10]=1;
maze.matrix[54][11]=1;
maze.matrix[54][12]=1;
maze.matrix[54][13]=1;
maze.matrix[54][14]=1;
maze.matrix[54][19]=1;
maze.matrix[54][22]=1;
maze.matrix[54][23]=1;
maze.matrix[54][24]=1;
maze.matrix[54][25]=1;
maze.matrix[54][28]=1;
maze.matrix[55][ 2]=1;
maze.matrix[55][ 6]=1;
maze.matrix[55][ 9]=1;
maze.matrix[55][14]=1;
maze.matrix[55][19]=1;
maze.matrix[55][22]=1;
maze.matrix[55][25]=1;
maze.matrix[55][28]=1;
maze.matrix[56][ 2]=1;
maze.matrix[56][ 3]=1;
maze.matrix[56][ 4]=1;
maze.matrix[56][ 5]=1;
maze.matrix[56][ 6]=1;
maze.matrix[56][ 7]=1;
maze.matrix[56][ 8]=1;
maze.matrix[56][ 9]=1;
maze.matrix[56][14]=1;
maze.matrix[56][19]=1;
maze.matrix[56][20]=1;
maze.matrix[56][21]=1;
maze.matrix[56][22]=1;
maze.matrix[56][25]=1;
maze.matrix[56][26]=1;
maze.matrix[56][27]=1;
maze.matrix[56][28]=1;
// number of total dots: 612; eating all of them means winning

//flashing dots
maze.matrix[28][2]=7;
maze.matrix[3][28]=7;
maze.matrix[56][2]=7;
maze.matrix[56][28]=7;

//pacman and mosters
	maze.matrix[pacman.x][pacman.y]=3;
	maze.matrix[M1.x][M1.y]=5;
	maze.matrix[M2.x][M2.y]=5;
	maze.matrix[M3.x][M3.y]=5;	
}
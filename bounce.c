#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/time.h>
#include<signal.h>
#include"bounce.h"
#include<curses.h>

struct ppball the_ball;

int baff_t;
int baff_len;

void set_up();
void wrap_up();
void ball_move(int);
int bounce_or_lose(struct ppball *hp);
void set_ticker(int m_secs);
void build_the_wall();

int main()
{
	int c;

	set_up();

	while ( (c=getchar()))
	{
		if (c == 'Q')
			break;
		mvaddch(LINES-1,COLS-1,c);
		if (c == 'f')
			the_ball.x_ttm--;
		else if (c == 'F')
			the_ball.y_ttm--;
		else if (c == 's')
			the_ball.x_ttm++;
		else if (c == 'S')
			the_ball.y_ttm++;
		else if ( c == 'j')
		{
			if (baff_t<(BOT_ROW-baff_len))
				baff_t++;
		}
		else if (c == 'k')
		{
			if (baff_t>TOP_ROW)
				baff_t--;
		}
	}

	wrap_up();
	return 0;
}

void set_up()
{
	the_ball.y_pos=Y_INIT;
	the_ball.x_pos=X_INIT;
	the_ball.y_ttm=Y_TTM;
	the_ball.x_ttm=X_TTM;
	the_ball.y_ttg=Y_TTM;
	the_ball.x_ttg=X_TTM;
	the_ball.y_dir=the_ball.x_dir=1;
	the_ball.symbol=DEF_SYMBOL;
	baff_t=TOP_ROW;
	baff_len=(BOT_ROW-TOP_ROW)/2;

	initscr();
	noecho();
	crmode();

//	signal(SIGINT,SIG_IGN);

	mvaddch(the_ball.y_pos,the_ball.x_pos,the_ball.symbol);
	build_the_wall();
	refresh();
	signal(SIGALRM,ball_move);
	set_ticker(1000/TICKS_PER_SEC);
}

void build_the_wall()
{
	int s_p;
	s_p=RIGHTEDGE-LEFTEDGE;
	char s[s_p];

	for (int i=0; i<s_p-1; i++)
		s[i]='#';
	s[s_p-1]='\0';

	move(TOP_ROW,LEFTEDGE);
	addstr(s);
	move(BOT_ROW,LEFTEDGE);
	addstr(s);

	for (int i=TOP_ROW; i<BOT_ROW; i++)
	{
		move(i,LEFTEDGE);
		addch('#');
		move(i,RIGHTEDGE);
		addch(' ');

	}

	for (int i=baff_t; i<baff_t+baff_len; i++)
	{
		move(i,RIGHTEDGE);
		addch('*');
	}
}
void wrap_up()
{
	set_ticker(0);
	endwin();
}

void set_ticker(int m_secs)
{
	struct itimerval the_time;
	long s_sec=m_secs/1000;
	long u_sec=(m_secs%1000)*1000;

	the_time.it_value.tv_sec=s_sec;
	the_time.it_value.tv_usec=u_sec;

	the_time.it_interval.tv_sec=s_sec;
	the_time.it_interval.tv_usec=u_sec;

	setitimer(ITIMER_REAL,&the_time,NULL);
}


void ball_move(int signum)
{
	int y_cur,x_cur,moved;

	y_cur=the_ball.y_pos;
	x_cur=the_ball.x_pos;
	moved=0;

	if (the_ball.y_ttm>0 && the_ball.y_ttg-- ==1)
	{
		the_ball.y_pos+=the_ball.y_dir;
		the_ball.y_ttg=the_ball.y_ttm;
		moved=1;
	}

	if (the_ball.x_ttm>0 && the_ball.x_ttg -- == 1)
	{
		the_ball.x_pos+=the_ball.x_dir;
		the_ball.x_ttg=the_ball.x_ttm;
		moved=1;
	}

	if (moved)
	{
		mvaddch(y_cur,x_cur,BLANK);
		mvaddch(the_ball.y_pos,the_ball.x_pos,the_ball.symbol);

		bounce_or_lose(&the_ball);

		build_the_wall();
		move(LINES-1,COLS-1);
		refresh();
	}
	signal(SIGALRM,ball_move);
}

int bounce_or_lose(struct ppball *hp)
{
	void lost();
	int return_val=1;

	if(hp->y_pos == TOP_ROW)
	{
		hp->y_dir=1;
		return_val=1;
	}
	else if (hp->y_pos == BOT_ROW)
	{
		hp->y_dir=-1;
		return_val=-1;
	}

	if (hp->x_pos == LEFTEDGE)
	{
		hp->x_dir=1;
		return_val=1;
	}
	else if (hp->x_pos == RIGHTEDGE)
	{
		if (hp->y_pos>=baff_t && (hp->y_pos <= (baff_t+baff_len)))
		{
			hp->x_dir=-1;
			return_val=1;
		}
		else
		{
			lost();
			return_val=0;
		}

	}

	return return_val;
}

void lost()
{
	move((TOP_ROW+BOT_ROW)/2,(LEFTEDGE+RIGHTEDGE)/3);
	addstr("Sorry,you've lost the game!");
	refresh();
	signal(SIGALRM,SIG_IGN);
	sleep(2);
	wrap_up();
	exit(0);
}

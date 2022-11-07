/*--------------------------------------------------------------------------*
 *----									----*
 *----		simplePinball.c						----*
 *----									----*
 *----	    This file defines most of the functionality needed to	----*
 *----	launch the bumper processes and play simple pinball.		----*
 *----									----*
 *----	----	----	----	----	----	----	----	----	----*
 *----									----*
 *----	Version 1a					Joseph Phillips	----*
 *----									----*
 *--------------------------------------------------------------------------*/

 
//	Compile with:
//	$ g++ simplePinball.c bounceBall.c -o simplePinball -lncurses -g


#include	"simplePinball.h"


//  PURPOSE:  To hold the integer 1 when the ball is still in play,
//	holds 0 otherwise.
int		isBallStillPlaying	= 1;


//  PURPOSE:  To hold the process ids of the child (bumper) processes.
pid_t		bumperPidArray[NUM_BUMPERS];


//  PURPOSE:  To handle when told that the ball has arrived at this process.
//	Ignores 'sigNum'.  No return value.
void		ballHandler	(int	sigNum
				)
{
  //  II.A.  Get rid of any chars already in buffer:
  while  (getch() != ERR);

  move(BALL_ROWS[0],(0+1) * BOUNCER_LEN + BOUNCER_LEN / 2);
  addch(BALL);
  refresh();
  usleep(TRANSIT_STEP_USECS);

  move(BALL_ROWS[0],(0+1) * BOUNCER_LEN + BOUNCER_LEN / 2);
  addch(NO_BALL);
  refresh();

  usleep(TRANSIT_STEP_USECS);

  if  (getch() == ERR)
  {
    isBallStillPlaying	= 0;
  }
  else
  {
    beep();
    move(BOUNCER_ROWS[0],BOUNCER_LEN);
    addstr(BOUNCED_BUMPER);
    refresh();

    usleep(RESPONSE_TIME_USECS);
    move(BOUNCER_ROWS[0],BOUNCER_LEN);
    addstr(FLIPPER);
    refresh();

    int	index	= rand() % NUM_BUMPERS;

    bounceBall(0,index+1,bumperPidArray[index]);
  }
  
}


//  PURPOSE:  To draw the table.  No parameters.  No return value.
void		drawTable	()
{
  //  I.  Application validity check:

  //  II.  Draw table:
  initscr();
  clear();
  noecho();
  cbreak();
  nodelay(stdscr,TRUE);

  int	col	= 2 * BOUNCER_LEN;

  for  (int i = 1;  i <= NUM_BUMPERS;  i++)
  {
    move(BOUNCER_ROWS[i],col);
    addstr(BUMPER);
    col	+= BOUNCER_LEN;
  }

  move(BOUNCER_ROWS[0],BOUNCER_LEN);
  addstr(FLIPPER);
  refresh();

  //  III.  Finished:
}


//  PURPOSE:  To start the child (bumper) processes.  No parameters.
//	No return value.
void		launchBumpers	()
{
  char	pidText[NUM_BUMPERS][LINE_LEN];

  for  (int i = 0;  i < NUM_BUMPERS;  i++)
  {
    bumperPidArray[i]	= fork();
    snprintf(pidText[i],LINE_LEN,"%d",bumperPidArray[i]);

    if  (bumperPidArray[i] == 0)
    {
      switch  (i)
      {
      case 0 :
        execl(BUMPER_PROGNAME,BUMPER_PROGNAME,NULL);
	break;

      case 1 :
        execl(BUMPER_PROGNAME,BUMPER_PROGNAME,pidText[0],NULL);
	break;

      case 2 :
        execl(BUMPER_PROGNAME,BUMPER_PROGNAME,pidText[0],pidText[1],NULL);
	break;
      }

      fprintf(stderr,"Cannot run %s\n",BUMPER_PROGNAME);
      exit(EXIT_FAILURE);
    }
  }

}


//  PURPOSE:  To run the programs as a whole.  Ignores command line arguments.
//	Returns 'EXIT_SUCCESS' to OS.
int		main		()
{
  //  I.  Application validity check:

  //  II.  Do pinball:
  //  II.A.  Install signal handlers:
  struct sigaction	action;

  memset(&action,'\0',sizeof(action));
  action.sa_handler	= ballHandler;
  sigaction(BALL_SIGNAL,&action,NULL);

  //  II.B.  Draw table:
  drawTable();

  //  II.C.  Launch bumpers:
  launchBumpers();

  //  II.D.  Play pinball:
  char	line[LINE_LEN];

  for  (int ballNum = NUM_BALLS;  ballNum > 0;  ballNum--)
  {
    isBallStillPlaying	= 1;

    move(0,BOUNCER_LEN);
    snprintf(line,LINE_LEN,"Press space to launch ball %d",ballNum);
    addstr(line);
    refresh();

    while  (getch() == ERR)
    {
      usleep(10000);
    }

    move(0,BOUNCER_LEN);
    addstr("                               ");
    move(BOUNCER_ROWS[0],BOUNCER_LEN);
    addstr(BOUNCED_BUMPER);
    refresh();

    usleep(RESPONSE_TIME_USECS);
    move(BOUNCER_ROWS[0],BOUNCER_LEN);
    addstr(NO_BUMPER);
    refresh();

    int	index	= rand() % NUM_BUMPERS;

    bounceBall(0,index+1,bumperPidArray[index]);

    while  (isBallStillPlaying)
    {
      sleep(1);
    }

  }

  //  II.E.  Tell bumpers to stop:
  endwin();

  for  (int i = 0;  i < NUM_BUMPERS;  i++)
  {
    kill(bumperPidArray[i],SIGINT);
    wait(NULL);
  }

  //  III.  Finished:
  return(EXIT_SUCCESS);
}

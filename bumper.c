/*--------------------------------------------------------------------------*
 *----									----*
 *----		bumper.c						----*
 *----									----*
 *----	    This file defines most of the functionality needed by the	----*
 *----	bumper processes.						----*
 *----									----*
 *----	----	----	----	----	----	----	----	----	----*
 *----									----*
 *----	Version 1a					Joseph Phillips	----*
 *----									----*
 *--------------------------------------------------------------------------*/

//	Compile with:
//	$ g++ bumper.c bounceBall.c -o bumper -lncurses

#include	"simplePinball.h"

//  PURPOSE:  To hold the process ids of the other known bouncers:
//	the parent process (the flipper), sibling bumpers created earlier,
//	and this process itself.
pid_t		bouncerPidArray[NUM_BUMPERS+1];


//  PURPOSE:  To tell the position of this process.
//	NOTE:	position 1 means the 1st bumper
//		position 2 means the 2nd bumper
//		position 3 means the 3rd bumper
int		thisPosition;


//  PURPOSE:  To hold 1 while this process should still run, or 0 otherwise.
int		shouldRun	= 1;


//  PURPOSE:  To note that this process should not run anymore.
//	Ignores 'sigNum'.  No return value.
void		intHandler	(int	sigNum
				)
{
  shouldRun	= 0;
}


//  PURPOSE:  To note that this process just received the ball.
//	Ignores 'sigNum'.  No return value.
void		ballHandler	(int	sigNum
				)
{
  {
    int	col	= 2 * BOUNCER_LEN;

    for  (int i = 1;  i <= NUM_BUMPERS;  i++)
    {
      move(BOUNCER_ROWS[i],col);
      addstr( (i == thisPosition) ? BOUNCED_BUMPER : BUMPER );
      col	+= BOUNCER_LEN;
    }

    move(BOUNCER_ROWS[0],BOUNCER_LEN);
    addstr(FLIPPER);
    refresh();
  }


  int	nextPosition	= rand() % (thisPosition+1);

  move(BALL_ROWS[0],(thisPosition+1) * BOUNCER_LEN + BOUNCER_LEN / 2);
  addch(BALL);
  refresh();
  usleep(TRANSIT_STEP_USECS);

  move(BALL_ROWS[0],(thisPosition+1) * BOUNCER_LEN + BOUNCER_LEN / 2);
  addch(NO_BALL);
  move(BOUNCER_ROWS[thisPosition],(thisPosition+1) * BOUNCER_LEN);
  addstr(BUMPER );
  refresh();
  bounceBall(thisPosition,nextPosition,bouncerPidArray[nextPosition]);
}


//  PURPOSE:  To runs the program as a whole.  'argc' tells the number of items
//	on the command line, and 'argv[]' has the array of those arguments.
//	Returns 'EXIT_SUCCESS' to the OS.
int		main		(int	argc,
				 char*	argv[]
				)
{
  //  I.  Application validity check:

  //  II.  Do bumpers:
  //  II.A.  Turn on graphics (already done):
  initscr();

  //  II.B.  Initialize 'bouncerPidArray[]' and 'thisPosition':
  bouncerPidArray[0]	= getppid();
  
  for  (int i = 1;  i < argc;  i++)
  {
    bouncerPidArray[i]	= strtol(argv[i],NULL,0);
  }

  bouncerPidArray[argc]	= getpid();
  thisPosition		= argc;

  //  II.C.  Install signal handler:
  struct sigaction	act;

  memset(&act,'\0',sizeof(act));
  act.sa_handler	= intHandler;
  sigaction(SIGINT,&act,NULL);

  act.sa_handler	= ballHandler;
  sigaction(BALL_SIGNAL,&act,NULL);

  //  II.D.  Hang out and bump:
  while  (shouldRun)
  {
    sleep(1);
  }

  //  III.  Finished:
  return(EXIT_SUCCESS);
}

#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Box.h>
#include <X11/Xmu/Editres.h>

/*
Compile instructions:
Add the dependencies for Xaw:
sudo apt install libxaw7-dev libxaw7
Compile and link:
gcc -o test2 test2.c -L/usr/X11R6/lib -lXaw -lXmu -lXt -lXext -lX11
Some info:
https://www.linuxquestions.org/questions/programming-9/capturing-the-window-close-event-in-x-358162/
*/

Atom wm_delete_window;

void XQuit(Widget w, XEvent *ev,String *vector, Cardinal *count);
void ButtonQuit(Widget w, XtPointer client_data, XtPointer call_data);
XtActionsRec Actions[] = {
    {"XQuit", XQuit}
};
int ActionsCount = XtNumber(Actions);

int main(int argc, char *argv[])
{
    Widget topwidget, container, quitButton;
    topwidget = XtInitialize(argv[0], "XSample", NULL, 0, &argc, argv);
    XtAddActions(Actions, ActionsCount);
    container = XtVaCreateManagedWidget("container", boxWidgetClass, topwidget, NULL);
    quitButton = XtVaCreateManagedWidget("quitButton", commandWidgetClass, container, XtNwidth, 180, NULL);
    XtAddCallback(quitButton, XtNcallback, ButtonQuit, NULL);
    XtRealizeWidget(topwidget);

    XtOverrideTranslations(
        topwidget, XtParseTranslationTable("<Message>WM_PROTOCOLS: XQuit()\n")
    );
        
    Display *dpy = XtDisplay(topwidget);
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, XtWindow(topwidget), &wm_delete_window, 1);

    XtMainLoop();
    return 0;
}

void XQuit(Widget w, XEvent *ev, String *vector, Cardinal *count)
{
    printf("close event by X ...\n");
    if (ev->type == ClientMessage &&
        ((Atom)ev->xclient.data.l[0]) == wm_delete_window)
        exit(0);
}

void ButtonQuit(Widget w, XtPointer client_data, XtPointer call_data)
{
    printf("close event by Quit button ...\n");
    exit(0);
}

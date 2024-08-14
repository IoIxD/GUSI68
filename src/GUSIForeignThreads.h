
#ifndef _GUSIForeignThreads_
#define _GUSIForeignThreads_

#define NewThread(style, entry, param, stack, options, result, made)	\
	GUSINewThread((style), (entry), (param), (stack), (options), (result), (made))
#define SetThreadSwitcher(thread, switcher, param, inOrOut) \
	GUSISetThreadSwitcher((thread), (switcher), (param), (inOrOut))
#define SetThreadTerminator(thread, threadTerminator, terminationProcParam) \
	GUSISetThreadTerminator((thread), (threadTerminator), (terminationProcParam))
#endif /* _GUSIForeignThreads_ */

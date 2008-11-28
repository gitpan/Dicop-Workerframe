/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for supporting of errors, warnings and other messages
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWD_MSG_H
#define DICOP_PWD_MSG_H

/* print out a given message */
void pwdgen_print_msg (const unsigned int msg);

#define PWDGEN_MESSAGES 3

#define ABORT_UNKNOWN_FILE pwdgen_print_msg(1); return PWD_ERROR;
#define ABORT_UNSUPPORTED_FILE pwdgen_print_msg(2); return PWD_ERROR;

#define ABORT_ON_IDENTIFY(pwd) if (pwd->identify != 0) { pwdgen_print_msg(3); return PWD_ERROR; }

#endif

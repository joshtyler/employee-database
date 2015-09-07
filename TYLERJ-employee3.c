/* 
	employee3.c v1.0
	Created by Joshua Tyler
	Based off a prototype by Nicolas Pugeault
	SOURCE CODE IS BEST VIEWED WITH A TAB WIDTH OF TWO
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Uncomment any of these lines to debug the respective sections */
/* #define DEBUG_READ_LINE */
/* #define DEBUG_READ_STRING */
/* #define DEBUG_PLACE_EMPLOYEE */
/* #define DEBUG_SEARCH_FOR_EMPLOYEE */
/* #define DEBUG_DELETE_EMPLOYEE */
/* #define DEBUG_END_OF_FILE_TEST */

/* Maximum length (in characters) that the respective structure members (which are strings) can be */
#define MAX_NAME_LENGTH 100
#define MAX_JOB_LENGTH  100

/* The maximum no. of characters to read into the buffer,
	 for the structure members that aren't stored as strings */
#define MAX_CHARS_TO_READ 300

/* Employee structure */
struct employee_struct
{
	/* Employee details */
	char name[MAX_NAME_LENGTH+1]; /* name string */
	char sex;                     /* sex identifier, either 'M' or 'F' */
	int  age;                     /* age */
	char job[MAX_JOB_LENGTH+1];   /* job string */
   
	/* pointers to previous and next employee structures in the linked list */
	struct employee_struct *prev, *next;
};

/* Typedef structure as 'employee' to make it easier to use */
typedef struct employee_struct employee;

/* Head pointer for linked list */
employee *head = NULL;

/* Global constants to make the use of the following arrays more intuitive */
#define PREFIX_OFF 0
#define PREFIX_ON 1
#define NAME_IDENTIFIER 0
#define SEX_IDENTIFIER 1
#define AGE_IDENTIFIER 2
#define JOB_IDENTIFIER 3

/* Array to store the prefixes that are used when reading input from a file and when outputting the database.

	 structure_member_prefix[PREFIX_OFF][NAME_IDENTIFIER] evaluates to a pointer to the string "\0", the same is true for AGE_IDENTIFIER etc.
	 This allows the same function (get_input) to be used both for reading input typed by the user (i.e without the user typing prompts) and input from a file (with prompts)
	 
	 structure_member_prefix[PREFIX_ON][NAME_IDENTIFIER] evaluates to a pointer to the string "Name: ", SEX_IDENTIFIER a pointer to the string "Sex: " etc. */
const char structure_member_prefix[2][4][7] = {{"\0"		 ,"\0"	 ,"\0"	 ,"\0"	 },
																							{"Name: ","Sex: ","Age: ","Job: "}};
/* Array to store the names of the elements in the structure.
	 This is used for producing helpful error messages for incorrect input.
	 structure_member_name[NAME_IDENTIFIER] evaluates to a pointer to the string "name", SEX_IDENTIFIER a pointer to the string "sex" etc. */
const char structure_member_name[4][5] = {"name","sex","age","job"};

/* An error message that is used multiple times in different functions */
const char *file_read_failure = "Failed to read database file, please ensure that it is formatted correctly.\n";

/* Global constants to make the use of print_error more intuitive*/
#define DO_NOT_EXIT 0
#define DO_EXIT 1

/* Global constants to make the use of get_input() and get_input_validity_check() more intuitive*/
#define INPUT_FROM_USER 0
#define INPUT_FROM_FILE 1

/* Function prototypes, function descriptions can be found with the function definitions */
static int read_line(FILE *fp, char *line, int max_length);
static int read_string(FILE *fp, const char *prefix, char *string, int max_length);
static void print_error(const char* string, int exit_status);
static void get_input_validity_check(int loop_count, int from_file, int field_identifier);
static employee *get_input(FILE *fp, int from_file);
static void print_single_employee(FILE *fp, const employee *employee_to_print);
static void place_employee(employee *employee_to_place);
static employee *search_for_employee(const char *name_to_delete);
static void delete_employee_from_list(employee *record_to_delete);
static int end_of_file_test(FILE *file_pointer);
static void menu_add_employee(void);
static void menu_print_database(void);
static void menu_delete_employee(void);
static void read_employee_database (const char *file_name);

/* codes for menu */
#define ADD_CODE    0
#define DELETE_CODE 1
#define PRINT_CODE  2
#define EXIT_CODE   3

/*
	Function: main()
	Purpose: A database program that allows the user to add employees, delete employees and print the database to the screen.
					 An existing database saved into a formatted file can also be loaded into the program.
	Arguments: The name of the database file to load (argv[1]).
	Return value: EXIT_SUCCESS (0) or EXIT_FAILURE (1)
	Inputs from user: Prompts to chose an option from the menu, and promts when inputting a new employee.
	Outputs to user: Prompts (printed to stderr)
									 All the employees indatabase (if the user selects that option from the menu), printed to stdout.
									 Relevant error messages (printed to stderr)
									 The program may exit without user promption, due to either a problem with the specified database file or a memory allocation error.
									 Debugging messages may be printed to stderr, if any of the #define lines are uncommented at the top of the source code.
 */
int main ( int argc, char *argv[] )
{
   /* check arguments */
   if ( argc != 1 && argc != 2 )
   {
      fprintf ( stderr, "Usage: %s [<database-file>]\n", argv[0] );
      exit(-1);
   }

   /* read database file if provided, or start with empty database */
   if ( argc == 2 )
      read_employee_database ( argv[1] );

   for(;;)
   {
      int choice, result;
      char line[301];

      /* print menu to standard error */
      fprintf ( stderr, "\nOptions:\n" );
      fprintf ( stderr, "%d: Add new employee to database\n", ADD_CODE );
      fprintf ( stderr, "%d: Delete employee from database\n", DELETE_CODE );
      fprintf ( stderr, "%d: Print database to screen\n", PRINT_CODE );
      fprintf ( stderr, "%d: Exit database program\n", EXIT_CODE );
      fprintf ( stderr, "\nEnter option: " );

      if ( read_line ( stdin, line, 300 ) != 0 ) continue;

      result = sscanf ( line, "%d", &choice );
      if ( result != 1 )
      {
	 fprintf ( stderr, "corrupted menu choice\n" );
	 continue;
      }

      switch ( choice )
      {
         case ADD_CODE: /* add employee to database */
	 menu_add_employee();
	 break;

         case DELETE_CODE: /* delete employee from database */
	 menu_delete_employee();
	 break;

         case PRINT_CODE: /* print database contents to screen
			     (standard output) */
	 menu_print_database();
	 break;

	 /* exit */
         case EXIT_CODE:
	 break;

         default:
	 fprintf ( stderr, "illegal choice %d\n", choice );
	 break;
      }

      /* check for exit menu choice */
      if ( choice == EXIT_CODE )
	 break;
   }

   return 0;   
}

/*
	Function: read_line()
	Purpose: Read a line of characters from a file pointer into a string.
					 A maximum number of characters to be read is specified.
					 A termination character '\0' is then added to the end of the string.
					 Reading stops upon encountering the end-of-line character '\n' (for which '\0' is substituted in the string.) 
					 If the line is longer than the maximum length "max_length" of the string, the extra characters are read but ignored.	 
	Arguments: The file pointer to read from (fp).
						 The string to store the output in (line)
						 The maximumum number of characters to read (max_length)
	Return value: 0 is returned upon successfully reading a line.
								-1 is returned if the end of file character EOF is reached before the end of the line.
	Inputs from user: None, unless the file pointer is stdin.
	Outputs to user: None, unless the "#define DEBUG_READ_LINE" line is uncommented at the top of the source code.
									 If it is the function will output each character it reads (and the corresponding ASCII code) to stderr.
									 (along with an explanatory message)
 */
static int read_line ( FILE *fp, char *line, int max_length )
{
	int i;
	char ch;

	/* initialize index to string character */
	i = 0;

	/* read to end of line, filling in characters in string up to its maximum length,
		 and ignoring the rest, if any */
	for(;;)
	{
		/* read next character */
		ch = fgetc(fp);

		#ifdef DEBUG_READ_LINE
		fprintf(stderr, "Read ascii %d which is a: %c ", ch, ch);
		#endif

		/* check for end of file error */
		if ( ch == EOF )
			return -1;

		/* check for end of line */
		if ( ch == '\n' )
		{
			/* terminate string and return */
			line[i] = '\0';
			return 0;
		}

		/* fill character in string if it is not already full*/
		if ( i < max_length )
			line[i++] = ch;
	}

	/* the program should never reach here */
	return -1;
}

/*
	Function: read_string()
	Purpose: Read a line of characters, which are preceeded with a set prefix, from a file pointer into a string.
					 The prefix is not copied onto the output string.
					 A maximum number of characters to be read is specified.
					 A termination character '\0' is then added to the end of the string.
					 Reading stops upon encountering the end-of-line character '\n' (for which '\0' is substituted in the string.) 
					 If the line is longer than the maximum length "max_length" of the string, the extra characters are read but ignored.	 
	Arguments: The file pointer to read from (fp).
						 The prefix which will preceed the string to read (prefix).
						 The string to store the output in (line).
						 The maximumum number of characters to read (max_length).
	Return value: 0 is returned upon successfully reading a line.
								-1 is returned if the end of file character EOF is reached before the end of the line.
								-1 is also returned if the prefix on the file pointer does not match the prefix specified.
	Inputs from user: None, unless the file pointer is stdin.
	Outputs to user: None, unless the "#define DEBUG_READ_STRING", or "#define DEBUG_READ_LINE" line is uncommented at the top of the source code.
									 If "#define DEBUG_READ_STRING" is uncommented the function will output each prefix character it reads (and the corresponding ASCII code) to stderr.
									 It will also have an explanatory message to say if each prefix character is "successful" (i.e the same as the specified prefix) or not.
									 If "#define DEBUG_READ_LINE" is uncommented the function will output as described in the read_line() specification.
									 This happens because read_string() calls read_line() to read the characters on the input buffer after the prefix.
 */

static int read_string ( FILE *fp, const char *prefix, char *string, int max_length )
{
	int i;
	char c;

	/* read prefix string */
	for ( i = 0; i < strlen(prefix); i++ )
		if ( (c = fgetc(fp)) != prefix[i] )
		{
			/* file input doesn't match prefix */
			#ifdef DEBUG_READ_STRING
			fprintf(stderr,"The failure character is: %c\nThis is an ascii code of: %d \n", c, c);#
			#endif
			return -1;
		}
		#ifdef DEBUG_READ_STRING
		else
			fprintf(stderr,"Success! the prefix character read was: %c\nThis is an ascii code of: %d \n", c, c);
		#endif
   /* read remaining part of line of input into string */
   return ( read_line ( fp, string, max_length ) );
}

/*
	Function: print_error()
	Purpose: Print a messge to stderr, and optionally exit the program.
					 (with the exit status EXIT_FAILURE).
	Arguments: The string to print (string).
						 An integer deterimining whether to exit the program (exit_status).
							This uses constants DO_NOT_EXIT and DO_EXIT defined above,
							but any integer that evaluates as FALSE (for DO_NOT_EXIT) or TRUE (for DO_EXIT) will work.
						 If exit_status evaluates as true, the program will exit, otherwise it won't.
	Return value: None.
	Inputs from user: None.
	Outputs to user: The error message that is printed, and (optionally) the fact that the program will terminate.
 */
static void print_error(const char* string, int exit_status)
{
	fputs(string,stderr);
	if(exit_status)
		exit(EXIT_FAILURE);
	return;
}

/*
	Function: get_input_validity_check()
	Purpose: Used during database input to print error messages (to stderr) and prompts (to stdout) at appropriate times.
					 The function will also exit if invalid input is found when reading from a file.
	Arguments: An integer containing how many times the user has already been prompted to enter that field (loop_count).
						 An integer deterimining whether the input is being taken from a file (from_file)
							This uses the constants defined at the top of the source code (INPUT_FROM_USER and INPUT_FROM_FILE),
							although will work by evaluating from_file as TRUE (for INPUT_FROM_FILE) or FALSE (for INPUT_FROM_USER) will work.
						 A field identifier, for determining which member of the structure is being input
							(uses the constants for structure members defined prevously)
	Return value: None.
	Inputs from user: None.
	Outputs to user: The error messages that are printed to stderr.
									 The prompts that are printed to stdout.
									 The fact that the program may terminate.
 */
static void get_input_validity_check(int loop_count, int from_file, int field_identifier)
{
	if( loop_count>0 && !(from_file) ){
		print_error("Invalid ", DO_NOT_EXIT);
		print_error(structure_member_name[field_identifier], DO_NOT_EXIT);
		print_error(", please enter again.\n", DO_NOT_EXIT);
	}else if( loop_count>0 && from_file ){
		print_error("Invalid ", DO_NOT_EXIT);
		print_error(structure_member_name[field_identifier], DO_NOT_EXIT);
		print_error(" found in database file, program will now exit.\n", DO_EXIT);
	}
	
	if(!from_file)
		fprintf(stderr, "%s", structure_member_prefix[PREFIX_ON][field_identifier]);
		
	return;
}

/*
	Function: get_input()
	Purpose: Used during database input to read the input from a file pointer, allocate memory for an employee structure and save the input to that structure.
					 If input is being read from the user, the program will loop until the user gives valid input.
					 If input if being read from a file, the program will exit if invalid input is given.
						An error message will be printed to stderr for both of the above cases.
					 If input is being read from the user, get_input will prompt the user for each structure member.
					 If input is being read from a file, get_input() will read (and ignore) the prompts specified in structure_member_prefix[][][] from the file.
	Arguments: A file pointer to read the input from (fp).
						 An integer determining if the input is from a file or from the user (from_file).
							This uses the constants INPUT_FROM_USER and INPUT_FROM_FILE, specified above.
							However any integer that evaluates as FALSE (for INPUT_FROM_USER) or TRUE (for INPUT_FROM_FILE) will work.
	Return value: A pointer to the employee structure that the input is saved in.
	Inputs from user: None.
	Outputs to user: The error messages that are printed to stderr.
									 The prompts that are printed to stderr.
									 The fact that the program may terminate if there is a problem allocating memory for the employee structure, or if there is invalid input.
 */
static employee *get_input(FILE *fp, int from_file)
{
	/* Allocate memory for an employee structure */
	employee *employee_input;
	employee_input = (employee *)malloc(sizeof(employee));
	
	/* If employee_input is NULL, the memory allocation failed. */
	if(employee_input == NULL)
		print_error("Problem allocating memory for another employee.\nThe program will now exit.\n", DO_EXIT);

	/* Buffer to temporarily store input for structure members that are not stored as strings */
	char buffer[MAX_CHARS_TO_READ + 1];
	
	/* A loop counter, for determining when to write the error messages */
	int loop_count;
	
	/* This for loop initially sets the first character of employee_input->name to '\0', meaning the string is empty.
		 It then loops until sscanf(employee_input->name,"%1[^\n]", buffer) returns 1, meaning that the user has entered something.
		 (Unless get_input_validity_check terminates the program, or read_string encounters EOF)
		 A prompt is given to the user (if the input is from the user not from a file) each time the loop executes.
		 An error message is given to the user each time invalid input is read. */
	for(employee_input->name[0] = '\0', loop_count=0; sscanf(employee_input->name,"%1[^\n]", buffer) < 1; loop_count++)
	{
		get_input_validity_check(loop_count, from_file, NAME_IDENTIFIER);
		if(read_string(fp, structure_member_prefix[from_file][NAME_IDENTIFIER], employee_input->name, MAX_NAME_LENGTH) == -1)
			print_error(file_read_failure, DO_EXIT);
	}

	/* This for loop initially sets employee_input->sex to '\0', so that it is definitely neither 'M' or 'F'
		 It then loops until only one character has been entered, and that character is either 'M' or 'F'
		 (Unless get_input_validity_check terminates the program, or read_string encounters EOF)
		 A prompt is given to the user (if the input is from the user not from a file) each time the loop executes.
		 An error message is given to the user each time invalid input is read. */
	for(employee_input->sex = '\0', buffer[1] = 'a', loop_count=0; (employee_input->sex != 'M') && (employee_input->sex != 'F'); loop_count++)
	{
		get_input_validity_check(loop_count, from_file, SEX_IDENTIFIER);
		if(read_string(fp, structure_member_prefix[from_file][SEX_IDENTIFIER], buffer, 2) == -1)
			print_error(file_read_failure, DO_EXIT);
			
		/* If this is TRUE, then only one character was entered */
		if(buffer[1] == '\0')
		{
			employee_input->sex = buffer[0];
			buffer[1] = 'a';
		}
	}

	/* This for loop initially sets employee_input->age to '-1', so that it is invalid.
		 It then loops until an integer greater than or equal to zero, with no extra characters is entered.
		 (Unless get_input_validity_check terminates the program, or read_string encounters EOF)
		 A prompt is given to the user (if the input is from the user not from a file) each time the loop executes.
		 An error message is given to the user each time invalid input is read. */
	for(employee_input->age = -1, loop_count=0; employee_input->age < 0; loop_count++)
	{
		get_input_validity_check(loop_count, from_file, AGE_IDENTIFIER);
		if(read_string(fp, structure_member_prefix[from_file][AGE_IDENTIFIER], buffer, MAX_CHARS_TO_READ) == -1)
			print_error(file_read_failure, DO_EXIT);
		/* If this evaluates as TRUE, then extra characters were entered after the number, so the input is invalid */
		if( sscanf(buffer,"%d%1[^\n]", &(employee_input->age), buffer) != 1 )
			employee_input->age = -1;

	}

	/* This for loop initially sets the first character of employee_input->job to '\0', meaning the string is empty.
		 It then loops until sscanf(employee_input->job,"%1[^\n]", buffer) returns 1, meaning that the user has entered something.
		 (Unless get_input_validity_check terminates the program, or read_string encounters EOF)
		 A prompt is given to the user (if the input is from the user not from a file) each time the loop executes.
		 An error message is given to the user each time invalid input is read. */
	for(employee_input->job[0] = '\0', loop_count=0; sscanf(employee_input->job,"%1[^\n]", buffer) < 1; loop_count++)
	{
		get_input_validity_check(loop_count, from_file, JOB_IDENTIFIER);
		if(read_string(fp, structure_member_prefix[from_file][JOB_IDENTIFIER], employee_input->job, MAX_JOB_LENGTH) == -1)
			print_error(file_read_failure, DO_EXIT);
	}

	/* Return the address of the employee structure containing the input */
	return employee_input;
}

/*
	Function: print_single_employee()
	Purpose: Output the details of a single employee record to a specified output stream
	Arguments: The stream to print the details of the employee onto (fp)
						 The address of the employee record to print (employee_to_print)
	Return value: None.
	Inputs from user: None.
	Outputs to user: The employee that is printed.
 */
static void print_single_employee(FILE *fp, const employee *employee_to_print)
{
	fprintf(fp, "%s%s\n", structure_member_prefix[PREFIX_ON][NAME_IDENTIFIER], employee_to_print->name);
	fprintf(fp, "%s%c\n", structure_member_prefix[PREFIX_ON][SEX_IDENTIFIER], employee_to_print->sex);
	fprintf(fp, "%s%d\n", structure_member_prefix[PREFIX_ON][AGE_IDENTIFIER], employee_to_print->age);
	fprintf(fp, "%s%s\n", structure_member_prefix[PREFIX_ON][JOB_IDENTIFIER], employee_to_print->job);
	return;
}

/*
	Function: place_employee()
	Purpose: Place an employee record into the correct place (i.e alphabetical order by name) in the linked list.
	Arguments: The address of the employee to add to the linked list.
	Return value: None.
	Inputs from user: None.
	Outputs to user: None, unless the "#define DEBUG_PLACE_EMPLOYEE" is uncommented at the top of the source code,
									 in which case the function outputs debugging messages and information at each stage of the sorting process.
 */
static void place_employee(employee *employee_to_place)
{
	/* Set temp_ptr (which we will use in the main part of place_employee to find the record that belongs directly before employee_to_place) to point at the head. */
	employee *temp_ptr = head;
	
	#ifdef DEBUG_PLACE_EMPLOYEE
	fprintf(stderr, "employee_to_place = %p, this points to:\n", employee_to_place);
	employee_to_place == NULL? fputs("Nothing.", stderr): print_single_employee(stderr, employee_to_place);
	#endif
	
	/* See if record belongs at the beginning of the list, if so make it the head */
	if(head == NULL)
	{
		#ifdef DEBUG_PLACE_EMPLOYEE
		fputs("The head is NULL, I am making this record the new head.\n\n", stderr);
		#endif
		
		head = employee_to_place;
		head->next = NULL;
		head->prev = NULL;
	/* See if the record belongs before the current head. If so, make it the new head */
	/* If the strcmp is > 0, the first string belongs AFTER the second */
	}else if(strcmp(head->name, employee_to_place->name) >= 0)
	{
		#ifdef DEBUG_PLACE_EMPLOYEE
		fputs("This record belongs before (or is the same as) the current head.\n\n", stderr);
		#endif
		
		head->prev = employee_to_place;
		employee_to_place->next = head;
		employee_to_place->prev = NULL;
		head = employee_to_place;
		
	}else{
		/* Otherwise the record must belong somewhere in the middle of the list */
		#ifdef DEBUG_PLACE_EMPLOYEE
		fputs("This record belongs somewhere after the head in the list.\n", stderr);
		fprintf(stderr, "strcmp goes to: %d, temp_ptr->next = %p\n", strcmp(head->name, employee_to_place->name), head->next);
		#endif
		
		/* This for loop finds the record directly before employee_to_place */
		for(temp_ptr = head; 1 ; temp_ptr = temp_ptr->next )
		{
			#ifdef DEBUG_PLACE_EMPLOYEE
			fprintf(stderr, "(internal to for loop)temp_ptr = %p, this points to:\n", temp_ptr);
			temp_ptr == NULL? fputs("Nothing.\n", stderr): print_single_employee(stderr, temp_ptr);
			#endif
			
			/* If this statement is true, then employee_to_place belongs one before temp_ptr */
			if(strcmp(employee_to_place->name, temp_ptr->name) <= 0)
			{
				temp_ptr = temp_ptr->prev;
				break;
			}

			/* If the current record pointed to by temp_ptr is at the end of the list, break and put the */
			if(temp_ptr->next == NULL)
				break;

		}
		/* temp_ptr now points to the entry that belongs directly before employee to place */
		
		#ifdef DEBUG_PLACE_EMPLOYEE
		printf("(after for loop, temp_ptr should now be directly before employee_to_place)temp_ptr = %p, this points to:\n", temp_ptr);
		temp_ptr == NULL? fputs("Nothing.\n", stderr): print_single_employee(stderr, temp_ptr);
		putchar('\n');
		#endif
		
		employee_to_place->prev = temp_ptr;
		employee_to_place->next = temp_ptr->next;
		
		/* We need to see if there is another record after temp_ptr, to avoid writing to NULL */
		if(temp_ptr->next != NULL)
			(temp_ptr->next)->prev = employee_to_place;
		temp_ptr->next = employee_to_place;
	}
	return;
}

/*
	Function: search_for_employee()
	Purpose: Find the first employee in the linked list whose name matches a given string.
	Arguments: A string containing the name of the employee to find (name_to_find).
	Return value: A pointer to the employee structure whose name matches the given string.
								A pointer to NULL will be returned if no employees match the given string.
	Inputs from user: None.
	Outputs to user: None, unless the "#define DEBUG_SEARCH_FOR_EMPLOYEE" line is uncommented at the top of the source code,
									 in which case the function outputs debugging messages and information at each stage of the searching process.
 */
static employee *search_for_employee(const char *name_to_find)
{
	/* Output structure */
	employee *current_record;

	/* If the list is empty, return NULL */
	if(head == NULL)
		return NULL;

	/* Loop through the linked list, until an employee whose name matches name_to_find is found (or we reach the end of th list) */
	for(current_record = head; current_record != NULL; current_record = current_record->next)
	{
		#ifdef DEBUG_SEARCH_FOR_EMPLOYEE
		fprintf(stderr, "(internal)current_record = %p, this points to:\n", current_record);
		current_record == NULL? fputs("Nothing.", stderr): print_single_employee(stderr, current_record);
		#endif
		
		/* If the name is found, break */
		if(strcmp(name_to_find, current_record->name) == 0)
				break;
	}
		#ifdef DEBUG_SEARCH_FOR_EMPLOYEE
		fprintf(stderr, "current_record = %p, this points to:\n", current_record);
		current_record == NULL? puts("Nothing.", stderr): print_single_employee(stderr, current_record);
		#endif
	
	/* current_record will now either contain the address of the matching employee,
			or NULL (since the next member of the last employee in the list is NULL) */
	return current_record;
}

/*
	Function: delete_employee_from_list()
	Purpose: Remove an employee from the linked list, rearranging the pointers of the surrounding employees so that the list remains intact.
	Arguments: A pointer to the employee record to remove.
	Return value: None.
	Inputs from user: None.
	Outputs to user: None, unless the "#define DEBUG_DELETE_EMPLOYEE" line is uncommented at the top of the source code,
									 in which case the function will output information about the employee it is about to delete
 */
static void delete_employee_from_list(employee *record_to_delete)
{
	#ifdef DEBUG_DELETE_EMPLOYEE
	printf("record_to_delete = %p\n record_to_delete->prev = %p\n record_to_delete->next = %p\n", record_to_delete, record_to_delete->prev, record_to_delete->next);
	#endif
	
	/* We have to check if the previous and next employees exist before we attempt to write to them */
	if(record_to_delete->prev != NULL)
		(record_to_delete->prev)->next = record_to_delete->next;
	if(record_to_delete->next != NULL)
		(record_to_delete->next)->prev = record_to_delete->prev;
	
	/* If we're removing the head, we need to make the head point to the next record in the list */
	if(record_to_delete	== head)
		head = record_to_delete->next;
		
	/* Free the space used by the record that we're deleting */
	free(record_to_delete);
	
	return;
}

/*
	Function: end_of_file_test()
	Purpose: Get the input file into the correct position to read the next record, and check to see if the file contains any more records.
	Arguments: A file pointer to the stream to manipulate (file_pointer).
	Return value: 0 if there are no more records on the file.
								1 if there are more records on the file.
	Inputs from user: None.
	Outputs to user: None, unless the "#define DEBUG_END_OF_FILE_TEST" line is uncommented at the top of the source code,
									 in which case the function will output each character it tests, the corresponding ASCII value,
									 and whether the function interprets that character as meaning that there is remaining records in the databae file.
 */
static int end_of_file_test(FILE *file_pointer)
{
	/* Integer to store the character that will be input */
	int c;
	/* Get the next character from the stream */
	c = fgetc(file_pointer);
	
	#ifdef DEBUG_END_OF_FILE_TEST
	fprintf(stderr, "End of file test character is: %c\nThis is an ascii code of: %d \n", c, c);
	#endif
	
	/* If the next character is NOT a \n, the database is incorrectly formatted */
	if(c == '\n'){
		/* If the character we got WAS a \n, try getting another to see if we're at the end of the file */
		c = fgetc(file_pointer);
		
		#ifdef DEBUG_END_OF_FILE_TEST
		fprintf(stderr, "Second end of file test character is: %c\nThis is an ascii code of: %d \n", c, c);
		#endif
		
	}else
		print_error("Database file is incorrectly formatted, the program will now exit.\n", DO_EXIT);

	/* If we did reach the end of the file when we got the second character, then return 0 */
	if(feof(file_pointer))
		return 0;
	/* Otherwise, "unget" the character (i.e put it back on the stream) and return 1 */
	ungetc(c, file_pointer);
	return 1;
}

/*
	Function: menu_add_employee()
	Purpose: A function,designed to be called from the menu system, that prompts the user to enter the details of a new employee.
					 It then places that employee in the correct place (alphabetical order by name field) in the linked list.
	Arguments: None.
	Return value: None.
	Inputs from user: The details of new employee.
	Outputs to user: Prompts for information and error messages (written to stderr).
									 This function may cause the program to close if there is a memory allocation error when allocating space for the new employee.
									 Additionally, debugging messages may be printed to stderr, if the relevant #define lines are uncommented at the top of the source code.
 */
static void menu_add_employee(void)
{
	employee *employee_to_add_ptr;

	employee_to_add_ptr = get_input(stdin, INPUT_FROM_USER);

	place_employee(employee_to_add_ptr);

	return;
}

/*
	Function: menu_print_database()
	Purpose: A function, designed to be called from the menu system, that prints all the employees in the database to stdout.
					 The employees are printed in alphabetical order, and with prefixes for each field (e.g. Name).
	Arguments: None.
	Return value: None.
	Inputs from user: None.
	Outputs to user: The details of all the employees in the database, writted to stdout.
									 Additionally, debugging messages may be printed to stderr, if the relevant #define lines are uncommented at the top of the source code.
 */
static void menu_print_database(void)
{
	employee *employee_to_print;

	for(employee_to_print = head; employee_to_print != NULL; employee_to_print = employee_to_print->next)
	{
		print_single_employee(stdout, employee_to_print);
		putchar('\n');
	}
	return;
}	    

/*
	Function: menu_delete_employee()
	Purpose: A function, designed to be called from the menu system, that deletes all the employees with a given name in the database.
	Arguments: None.
	Return value: None.
	Inputs from user: None.
	Outputs to user: The name of the employee(s) to delete
									 Debugging messages may be printed to stderr, if the relevant #define lines are uncommented at the top of the source code.
 */
static void menu_delete_employee(void)
{
	/* Declare a string containing the name of the employee to delete, and a pointer to the employee to delete */
	char employee_name_to_delete[MAX_NAME_LENGTH + 1];
	employee *employee_to_delete;
	
	/* Prompt the user to enter the name of the employee(s) they would like to delete */
	fputs("Please enter the name of the employee to be deleted: ", stderr);
	read_line(stdin, employee_name_to_delete, MAX_NAME_LENGTH);
	
	/* Search for the employee to delete */
	employee_to_delete = search_for_employee(employee_name_to_delete);

	if(employee_to_delete == NULL)
	{
		fputs("Employee not found.\n", stderr);
		return;
	}

	delete_employee_from_list(employee_to_delete);

	/* This loop keeps removing employees whose name match the string specified, until no employees remain whose name match the string specified. */
	for(employee_to_delete = search_for_employee(employee_name_to_delete); employee_to_delete != NULL; employee_to_delete = search_for_employee(employee_name_to_delete))
		delete_employee_from_list(employee_to_delete);

	return;
}

/*
	Function: read_employee_database()
	Purpose: A function, which is run upon starting the program (if a database file is specified in the program arguments),
					 that loads the employees from a formatted database file into the database.
	Arguments: The name of the database file to load (file_name).
	Return value: None.
	Inputs from user: None.
	Outputs to user: Relevant error messages (printed to stderr)
									 The program may exit, if there is a problem with the database file (i.e file not found, or incorrect formatting.
									 Debugging messages may be printed to stderr, if the relevant #define lines are uncommented at the top of the source code.
 */
static void read_employee_database(const char *file_name)
{
	/* File pointer for the database file */
	FILE *file_pointer;

	/* Attempt to open the file specified by the user */
	file_pointer = fopen(file_name, "r");

	/* If the file pointer is NULL, the file couldn't be opened. */
	if(file_pointer == NULL)
		print_error("Error opening database file.\nThe program will now exit.\n", DO_EXIT);

	/* Pointer to employee structure for storing the address of the employee records as they are read from the file. */
	employee *current_employee_ptr;

	/* Loop through the file, reading each employee into an employee structure and sorting it into the linked list.
		 Stop when the end of the file is reached. */
	do{
		current_employee_ptr = get_input(file_pointer, INPUT_FROM_FILE);
		place_employee(current_employee_ptr);
	} while(end_of_file_test(file_pointer));

	/* Close the file */
	fclose(file_pointer);

	return;
}


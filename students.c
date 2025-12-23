#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define DATA_FILE "" // ENTER YOUR DATA FILE
#define MENU_OPTIONS 4

const char* menu_options[] = {"DISPLAY STUDENTS", "MANAGE STUDENTS", "MODIFY GRADES", "QUIT"};

enum select_options {
  DISPLAY_STUDENTS,
  MANAGE_STUDENTS,
  MODIFY_GRADES,
  QUIT
};

struct student{
	char name[32];
	char surname[32];
	uint16_t age;
	uint16_t grades_counter;
	uint8_t grades[32];
};

struct student *students = NULL;
int students_index = 0;

void add_student(void);
void delete_student(void);
void display_students(void);
void cstrcpy(char* dst, const char* src);
int is_int(char* s);
int is_digit(char c);
int ctoi(char c);
void print_menu(int selected);
void print_student_menu(void);
void print_grade_menu(void);
int select_student(void);
void add_grade(const int student_number);
void modify_grade(const int student_number);
char getch(void);
void get_enter(void);
void load_data(void);

int main(void) {
        load_data();
        
        enum select_options selected = 0;
	char input;

	print_menu(selected);
	while(1){
	        printf("\033[H\033[J"); // move cursor to the top-left & clear screen
		printf("\033[?25l");    // hide cursor
		print_menu(selected);
		
		input = getch();

		if (input == 'w'){
			 if (selected > 0) selected = (selected - 1 ) % MENU_OPTIONS;
			else selected = MENU_OPTIONS - 1;
		}
		else if (input == 's' || input == '\t') selected = (selected + 1) % MENU_OPTIONS;
		else if (input == '\n'){
			switch(selected){
				case DISPLAY_STUDENTS: display_students();
				                       print_menu(selected);
				                       break;
				case MANAGE_STUDENTS: print_student_menu();
					              print_menu(selected);
					              break;
				case MODIFY_GRADES: print_grade_menu();
					            print_menu(selected);
				                    break;
				case QUIT: printf("\033[?25h");
					   return 0;
				default: return -1;
			}
		}
	}
	free(students);
	printf("\033[?25h");
	return 0;
}

void add_student(void){
	char name[32];
	char surname[32];
	char buff_age[8];
	uint8_t age = 0;
        int c;

	printf("\033[H\033[J"); // clear screen
	printf("\033[?25h");    // show cursor
	printf("\n  Enter the students's name: ");
	scanf("%s", name);
        printf("  Enter the students's surname: ");
	scanf("%s", surname);

        do {
          printf("  Enter student's age: ");
          scanf("%s", buff_age);
          // Clear input buffer
          while ((c = getchar()) != '\n' && c != EOF);
          if (is_int(buff_age)) age = atoi(buff_age);
        } while(!age);

        char grade[16];
        uint8_t grade_n = 0;
        do{
            printf("  Enter grade(1-5): ");
            fgets(grade, sizeof(grade), stdin);
            grade[strcspn(grade, "\n")] = '\0';
            if (strlen(grade) == 0) break;
            else grade_n = atoi(grade);
        } while((grade_n <= 0 || grade_n > 5));

        void *tmp = realloc(students, (students_index + 1) * sizeof(struct student));
        if (tmp != NULL)
          students = tmp;
        else {
        }

	strcpy(students[students_index].name, name);
	strcpy(students[students_index].surname, surname);
	students[students_index].age = age;

	if(grade_n > 0 && grade_n <= 5){
	    students[students_index].grades[0] = grade_n;
	    students[students_index].grades_counter = 1;
	}
	students_index++;

        // SAVE TO FILE
        FILE *dat_file = fopen(DATA_FILE, "a");
        if (grade_n == 0)
          fprintf(dat_file, "STUDENT_%d\n{\nNAME:%s\nSURNAME:%s\nAGE:%d\nGRADES:\n}\n", students_index, name, surname, age);
        else
          fprintf(dat_file, "STUDENT_%d\n{\nNAME:%s\nSURNAME:%s\nAGE:%d\nGRADES:%d\n}\n", students_index, name, surname, age, grade_n);
        fclose(dat_file);

	printf("\n  \033[1;31mStudent '%s %s' has been added to system\033[0m", students[students_index - 1].name, students[students_index - 1].surname);
	printf("\033[?25l");  // hide cursor
	fflush(stdout);       // force output
	get_enter();
}

void delete_student(void){
    int student_number = select_student();
    char name[32];
    char surname[32];
    strcpy(name, students[student_number].name);
    strcpy(surname, students[student_number].surname);
    
    // user pressed Q 
    if (student_number < 0) return; 
    
    for(int i = student_number; i < students_index - 1 ; i++){
        students[i] = students[i + 1]; 
    }
    
    if (students_index - 1 != 0) {
      void *tmp = realloc(students, (students_index - 1) * sizeof(struct student));
      if (tmp != NULL)
        students = tmp;
    }
    else
      students = realloc(students, 0);

    students_index--;
    
    // MODIFY FILE
    char buffer[64];
    FILE *tmp_file = fopen("tmp.dat", "w");
    FILE *dat_file = fopen(DATA_FILE, "r");

    char student_to_remove[32];
    sprintf(student_to_remove, "STUDENT_%d\n", student_number + 1);
    
    while(fgets(buffer, sizeof(buffer), dat_file) != NULL){
      if (!strcmp(buffer, student_to_remove)){
        while(fgets(buffer, sizeof(buffer), dat_file) != NULL) {
          if (!strcmp(buffer, "}\n"))
            break;
        }
      }
      else {
        char num_a[16];
        int i = 8;
        if (!strncmp(buffer, "STUDENT_", 8)) {
          for(i; buffer[i] != '\n'; i++)
            num_a[i - 8] = buffer[i];
          num_a[i - 8] = '\0';
          int num = atoi(num_a);
          
          if (num > student_number + 1)
            sprintf(buffer, "STUDENT_%d\n", num - 1);
        }
        fprintf(tmp_file, "%s" ,buffer);
      }
    }
    
    fclose(dat_file);
    fclose(tmp_file);
    remove(DATA_FILE);
    rename("tmp.dat", DATA_FILE);
    
    printf("\n  \033[1;31mStudent '%s %s' has been deleted from the system\033[0m", name, surname);
    get_enter();
}

void display_students(void){
    printf("\033[H\033[J"); // clear screen
    if (students_index == 0) {
      printf("\n  0 results");
      printf("\n\n  \033[1;32mPress ENTER to return to MENU\033[0m");
      get_enter();
    }
    
    else {
      printf("\n  \033[1m%d registred students\033[0m\n", students_index);
      for(int i = 0; i < students_index; i++) {
          printf("\n  \033[4mStudent %d:\033[0m\n  \tName: %s\n  \tSurname: %s\n  \tAge: %d\n  \tGrades: ", i + 1, students[i].name, students[i].surname, students[i].age);
        for(int j = 0; j < students[i].grades_counter; j++){
            if(j + 1 != students[i].grades_counter) printf("%d, ", students[i].grades[j]);
            else printf("%d", students[i].grades[j]);
        }
      }
      printf("\n\n  \033[1;32mPress ENTER to return to MENU\033[0m");
      
      char cur_input;
      char input[32];
      uint8_t input_c = 0;
      
      while ((cur_input = getch()) != '\n') {
        if (cur_input == 0x7F) {
          if (input_c > 0)
            input[--input_c] = '\0';
        }
        else
          input[input_c++] = tolower(cur_input);
        
        printf("\033[H\033[J");
        printf("  %s\n", input);
        
        for(int i = 0; i < students_index; i++) {
          char name[32];
          strcpy(name, students[i].name);
          for (int i = 0; i < strlen(name); i++)
            name[i] = tolower(name[i]);
          if (!strncmp(input, name, input_c)) {
            printf("\n  \033[4mStudent %d:\033[0m\n  \tName: %s\n  \tSurname: %s\n  \tAge: %d\n  \tGrades: ", i + 1, students[i].name, students[i].surname, students[i].age);
            for(int j = 0; j < students[i].grades_counter; j++){
                if(j + 1 != students[i].grades_counter) printf("%d, ", students[i].grades[j]);
                else printf("%d", students[i].grades[j]);
            }
          }
        }
        printf("\n\n  \033[1;32mPress ENTER to return to MENU\033[0m");
      }
    }
}

/*
void display_students(void){
    printf("\033[H\033[J"); // clear screen
    if (students_index == 0) printf("\n  0 REGISTRED STUDENTS");
    else {
      printf("\n  \033[1m%d registred students\033[0m\n", students_index);
      for(int i = 0; i < students_index; i++) {
          printf("\n  \033[4mStudent %d:\033[0m\n  \tName: %s\n  \tSurname: %s\n  \tAge: %d\n  \tGrades: ", i + 1, students[i].name, students[i].surname, students[i].age);
          for(int j = 0; j < students[i].grades_counter; j++){
              if(j + 1 != students[i].grades_counter) printf("%d, ", students[i].grades[j]);
              else printf("%d", students[i].grades[j]);
          }
      }
    }
    printf("\n\n  \033[1;32mPress ENTER to return to MENU\033[0m");
    get_enter();
}
*/

void print_student_menu(void){

    char* options[] = {"ADD STUDENT", "DELETE STUDENT"};
    int choise = 0;
    char input;

   do
   {
      printf("\033[H\033[J\n");   
      if (choise == 0) printf("\033[1;31m\t> %s\033[0m   %s", options[0], options[1]);
      else printf("\t  %s\033[1;31m > %s\033[0m", options[0], options[1]);
      
      puts("\n\n\n\033[1;32m  Q = BACK TO MENU\033[0m");
      
      input = getch();
      if(input == 'd') choise = (choise + 1) % 2;
      else if(input == 'a') choise = (choise - 1) % 2;
      else if(input == 'q') return;
      
   } while(input != '\n');
  
  if (choise == 0) 
    add_student();
  else 
    delete_student();

}

void print_grade_menu(void){
    int choise = 0;
    int selected = 0;
    char input;
    char* options[] = {"ADD GRADES", "MODIFY GRADE"};
    
    do{
    
        printf("\033[H\033[J\n"); // clear screen.
        if (choise == 0) printf("\033[1;31m\t> %s\033[0m   %s", options[0], options[1]);
        else printf("\t  %s\033[1;31m > %s\033[0m", options[0], options[1]);
        printf("\n\n\n\033[1;32m  Q = BACK TO MENU\033[m");
        
        input = getch();
        if (input == 'd') choise = (choise + 1) % 2;
        else if(input == 'a') choise = (choise - 1) % 2;
        else if(input == 'q') return;
        
    } while(input != '\n');
    
    int selected_student = select_student();
    if(selected_student < 0) return;
        
    if(choise == 0){
        add_grade(selected_student);
        get_enter();
    }
    else modify_grade(selected_student);
}

int select_student(void){
        int selected = 0; 
        char input;
        
        do{
            printf("\033[H\033[J"); // clear screen
            printf("\n\033[1;32m  Q = BACK TO MENU\033[0m");
            printf("\n\n  Choose student:\n\n");
                for(int i = 0; i < students_index; i++){
                    if(i == selected) printf("\033[1;31m> %d: %s %s\033[0m\n", i + 1, students[i].name, students[i].surname);
                    else printf("  %d: %s %s\n", i + 1, students[i].name, students[i].surname);
                }
                
            input = getch();
            if(input == 'w'){
                if(selected > 0) selected = (selected - 1);
                else selected = students_index - 1;
            }
            else if(input == 's') selected = (selected + 1) % students_index;
            else if(input ==  'q') return -1;

        } while(input != '\n');
        
        return selected;
}

void add_grade(const int s_num){
    int grades[32];
    int grades_counter = 0;
    char grades_input[64];

    printf("\033[H\033[J\033[?25h"); // clear screen & show cursor
    printf("\n  SELECTED STUDENT: \033[1;31m%s %s\033[0m\n", students[s_num].name, students[s_num].surname);
    
    printf("  GRADES TO ADD: ");
    fgets(grades_input, sizeof(grades_input), stdin);
    grades_input[strcspn(grades_input, "\n")] = '\0';
    char *token = strtok(grades_input, " ");
    while(token != NULL){
        if (atoi(token) >= 1 && atoi(token) <= 5)
            grades[grades_counter++] = atoi(token);
        token = strtok(NULL, " ");
    }
    
    printf("\n\033[1;31m  GRADES {");
    for (int i = 0; i < grades_counter; i++){
        students[s_num].grades[students[s_num].grades_counter++] = grades[i];
        if (i + 1 != grades_counter) printf("%d, ", grades[i]);
        else printf("%d", grades[i]);
    }
    printf("} ADDED TO STUDENT '%s %s'\033[0m", students[s_num].name, students[s_num].surname);

    FILE *dat_file = fopen(DATA_FILE, "r");
    FILE *tmp_file = fopen("tmp.dat", "w");
    char buffer[128];
    char modified_s[32];
    sprintf(modified_s, "STUDENT_%d\n", s_num + 1);
    
    while(fgets(buffer, sizeof(buffer), dat_file)) {
      if(!strcmp(buffer, modified_s)) {
        fprintf(tmp_file, "%s", buffer);
        do {
          fgets(buffer, sizeof(buffer), dat_file);
          
          if (!strncmp(buffer, "GRADES", 6)) {
            fprintf(tmp_file, "%s", "GRADES:");
            for(int i = 0; i < students[s_num].grades_counter; i++)
              if (i + 1 != students[s_num].grades_counter)
                fprintf(tmp_file, "%c ", (char) (students[s_num].grades[i]) + '0');
              else
                fprintf(tmp_file, "%c\n", (char) (students[s_num].grades[i]) + '0');
          }
          else {
            fprintf(tmp_file, "%s", buffer);
          }
        } while(strcmp(buffer, "}\n"));
        continue;
      }
      fprintf(tmp_file, "%s", buffer);
    }
    
    fclose(dat_file);
    fclose(tmp_file);
    remove(DATA_FILE);
    rename("tmp.dat", DATA_FILE);
}

void modify_grade(const int s_num){
    int selected_grade = 0;
    char input;

    do{
        do{
            printf("\033[H\033[J"); // clear screen
            printf("\n  SELECTED STUDENT: \033[1;31m%s %s\033[0m\n  GRADES { ", students[s_num].name, students[s_num].surname);
            
            for(int i = 0; i < students[s_num].grades_counter; i++){
                if(selected_grade != i) printf("  %d  ", students[s_num].grades[i]);
                else printf("\033[1;31m> %d  \033[0m", students[s_num].grades[i]);
            }
            puts("}\n\n\n\033[0;32m  ENTER = SELECT\n  Q = BACK TO MENU\033[0m");
            
            input = getch();
            if(input == 'd') selected_grade = (selected_grade + 1) % students[s_num].grades_counter;
            else if(input == 'a'){
                if(selected_grade != 0) selected_grade = (selected_grade - 1) % students[s_num].grades_counter;
                else selected_grade = students[s_num].grades_counter - 1;
            }
            else if(input == 'q') return;
            
        }while(input != '\n');
        
        printf("\033[H\033[J"); // clear screen
        printf("\n  SELECTED STUDENT: \033[1;31m%s %s\033[0m\n  GRADES { ", students[s_num].name, students[s_num].surname);
            
        for(int i = 0; i < students[s_num].grades_counter; i++){
            if(selected_grade != i) printf("  %d  ", students[s_num].grades[i]);
            else printf("\033[1;5;31m> %d  \033[0m", students[s_num].grades[i]);
        }
        puts("}\n\n\n\033[0;32m  NUMBER <1;5> = CHANGE GRADE\n  Q / ENTER = BACK TO MENU\033[0m");
        
        input = getch();
        if (input >= '1' && input <= '5') {
            students[s_num].grades[selected_grade] = ctoi(input);
        }
        else if (input == 'q') break;
        
    } while(input != '\n');
    
    
    
    FILE *dat_file = fopen(DATA_FILE, "w");
    char buffer[64];
    
    for (int i = 0; i < students_index; i++) {
      sprintf(buffer, "STUDENT_%d\n{\n", i + 1);
      fprintf(dat_file, "%s", buffer);
      
      sprintf(buffer, "NAME:%s\n", students[i].name);
      fprintf(dat_file, "%s", buffer);
      
      sprintf(buffer, "SURNAME:%s\n", students[i].surname);
      fprintf(dat_file, "%s", buffer);
      
      sprintf(buffer, "AGE:%d\n", students[i].age);
      fprintf(dat_file, "%s", buffer);
      
      fprintf(dat_file, "%s", "GRADES:");
      for(int j = 0; j < students[i].grades_counter; j++) {
        if (j + 1 != students[i].grades_counter)
          fprintf(dat_file, "%d ", students[i].grades[j]);
        else
          fprintf(dat_file, "%d\n}\n", students[i].grades[j]);
      }
    }
}

int is_int(char *s){
	while(*s){
		if (!is_digit(*s)) return 0;
		s++;
	}
	return 1;
}

int is_digit(char c){
	if (c >= '0' && c <= '9') return 1;
	return 0;
}

int ctoi(char c){
    int n = c - '0';
    return n;
}

void print_menu(int selected){
	printf("\033[H\033[J");
	printf("\n  What do you wanna do? \n");
	for(int i = 0; i < MENU_OPTIONS; i++){
		if(i == selected) printf("\033[1;31m> %s\033[0m\n", menu_options[i]);
		else printf("  %s\n", menu_options[i]);
	}
}

// disables echo when inputing, reads one character without buffering from stdin then restores previous settings
// returns read character
char getch(void) {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// disables echo when inputting, function runs until ENTER is pressed, then restores previous settings
void get_enter(void){
	struct termios oldt, newt;
	int c;
	// Save current terminal settings
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	// Disable echo, keep canonical mode (so Enter is still required)
	newt.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	// Read until Enter is pressed
	while ((c = getchar()) != '\n' && c != EOF);
	// Restore old settings
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void load_data(void) {

  FILE *dat_file = fopen(DATA_FILE, "r");
  if(dat_file == NULL)
    return;

  char buffer[64];
  char num_placeholder[4];
  uint16_t num = 0;

  while(fgets(buffer, sizeof(buffer), dat_file) != NULL) {
    if (!strncmp(buffer, "STUDENT_", 8)) {
      students_index++;
      uint8_t i = 8;
      for(i; buffer[i] != '\n'; i++)
        num_placeholder[i - 8] = buffer[i];
      num_placeholder[i - 8] = '\0';
      num = atoi(num_placeholder) - 1;
      
      students = realloc(students, sizeof(struct student) * (num + 1));
      
      while(1) {
        fgets(buffer, sizeof(buffer), dat_file);
        if (!strncmp(buffer, "}" ,1))
          break;
        
        if (!strncmp(buffer, "NAME", 4)) {
          char name[64];
          for(i = 5; buffer[i] != '\n'; i++)
            name[i - 5] = buffer[i];
          name[i - 5] = '\0';
          strcpy(students[num].name, name);
        }
        else if(!strncmp(buffer, "SURNAME", 7)) {
          char surname[64];
          for(i = 8; buffer[i] != '\n'; i++)
            surname[i - 8] = buffer[i];
          surname[i - 8] = '\0';
          strcpy(students[num].surname, surname);
        }
        else if(!strncmp(buffer, "AGE", 3)) {
          char age[4];
          for(i = 4; buffer[i] != '\n'; i++)
            age[i - 4] = buffer[i];
          students[num].age = atoi(age);
        }
        else if(!strncmp(buffer, "GRADES", 6)) {
          char grade[2];
          uint8_t grade_c = 0;
          for(i = 7; buffer[i] != '\n'; i++) {
            if (buffer[i] != ' ') {
              grade[0] = buffer[i];
              grade[1] = '\0';
              students[num].grades[grade_c++] = atoi(grade);
            }
          }
          students[num].grades_counter = grade_c;
        }
      }
    }
  }
}

//Urse Andrei 312CB
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define FILE_NAME 100
#define RULE_SIZE 100
#define SIZE 256
#define CMD_LEN 200
#define PI 3.14159265358979323846

//L-System Struct
typedef struct {
	char symbol;
	char successor[RULE_SIZE];
} rule;

typedef struct {
	char axiom[SIZE];
	int nrules;
	rule rules[RULE_SIZE];
	int loaded;//status flag
} lsystem;

//Image Structs
typedef struct {
	unsigned char r, g, b;
} pixel;

typedef struct {
	int width, height;
	pixel *pixel_mat;
	int loaded;//status flag
} image;

typedef struct {
	double x, y, orient;
} turtle;

//UNDO/REDO Structs

//Singly Linked List Struct
typedef struct list {
	char phrase[CMD_LEN];
	struct list *next;
} list;

//History State Struct
typedef struct {
	lsystem lsys;
	image img;
	list *head;
	int list_length;
	int current_cmds;
	int silent;//(UNDO -> silent = 1; REDO -> silent = 0)
} state;

//L-SYSTEM Function
void load_lsystem(state *snap, char *lsys_file)
{
	FILE *f = fopen(lsys_file, "rt");
	if (!f) {
		printf("Failed to load %s\n", lsys_file);
		snap->lsys.nrules = 0;
		snap->lsys.loaded = 0;
		return;
	}

	fscanf(f, "%s %d", snap->lsys.axiom, &snap->lsys.nrules);
	for (int i = 0; i < snap->lsys.nrules; i++) {
		fscanf(f, " %c %s", &snap->lsys.rules[i].symbol,
			   snap->lsys.rules[i].successor);
	}
	if (!snap->silent) {
		printf("Loaded %s (L-system with %d rules)\n",
			   lsys_file, snap->lsys.nrules);
	}
	snap->lsys.loaded = 1;
	fclose(f);
}

//DERIVE Helper Functions
char *replace(lsystem *lsys, char symbol)
{
	//Replace key with corresponding string
	for (int i = 0; i < lsys->nrules; i++) {
		if (lsys->rules[i].symbol == symbol) {
			return lsys->rules[i].successor;
		}
	}
	return NULL;
}

char *generate_rule(lsystem *lsys, int cnt)
{
	char *str = malloc(strlen(lsys->axiom) + 1);
	if (!str) {
		perror("Error allocating memory");
		return NULL;
	}
	strcpy(str, lsys->axiom);

	for (int i = 0; i < cnt; i++) {
		//Calculate new string's length
		int len = 0;
		for (int k = 0; str[k]; k++) {
			char *successor = replace(lsys, str[k]);
			if (successor) {
				len += strlen(successor);
			} else {
				len += 1;
			}
		}
		//Allocate memory and build new string
		char *new_str = malloc(len + 1);
		if (!new_str) {
			perror("Error allocating memory\n");
			free(str);
			return NULL;
		}
		char *p = new_str;
		for (int k = 0; str[k]; k++) {
			char *successor = replace(lsys, str[k]);
			if (successor) {
				strcpy(p, successor);
				p += strlen(successor);
			} else {
				*p = str[k];
				p++;
			}
		}
		*p = '\0';
		free(str);
		str = new_str;
	}
	return str;
}

//DERIVE Function
void derive(lsystem *lsys)
{
	int cnt;
	scanf("%d", &cnt);

	//Verify if a L-System has been loaded
	if (lsys->loaded == 0) {
		printf("No L-system loaded\n");
		return;
	}

	char *result = generate_rule(lsys, cnt);
	printf("%s\n", result);
	free(result);
}

//LOAD Function
void load_image(state *snap, char *img_file)
{
	FILE *f = fopen(img_file, "rb");
	if (!f) {
		printf("Failed to load %s\n", img_file);
		snap->img.loaded = 0;
		return;
	}

	//Verify if another image is already loaded
	if (snap->img.loaded) {
		free(snap->img.pixel_mat);
	}

	char format[3];
	int max_val;
	fscanf(f, "%s %d %d %d", format,
		   &snap->img.width, &snap->img.height, &max_val);
	fgetc(f);

	snap->img.pixel_mat = malloc(snap->img.width *
								 snap->img.height * sizeof(pixel));
	snap->img.loaded = 1;

	fread(snap->img.pixel_mat, sizeof(pixel),
		  snap->img.width * snap->img.height, f);

	if (!snap->silent)
		printf("Loaded %s (PPM image %dx%d)\n", img_file,
			   snap->img.width, snap->img.height);
	snap->img.loaded = 1;
	fclose(f);
}

//Bresenham Algorithm
void draw_line(image *img, int x0, int y0, int x1, int y1, int r, int g, int b)
{
	int dx = abs(x1 - x0), dy = -abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;

	while (1) {
		if (x0 >= 0 && x0 < img->width && y0 >= 0 && y0 < img->height) {
			int index = (img->height - 1 - y0) * img->width + x0;
			img->pixel_mat[index].r = (unsigned char)r;
			img->pixel_mat[index].g = (unsigned char)g;
			img->pixel_mat[index].b = (unsigned char)b;
		}
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}
}

//Turtle Graphics
void run_turtle(state *snap, char *parameters)
{
	if (!snap->lsys.loaded) {
		if (!snap->silent) {
			printf("No L-system loaded\n");
		}
		return;
	}
	if (!snap->img.loaded) {
		if (!snap->silent) {
			printf("No image loaded\n");
		}
		return;
	}

	turtle t;
	double steps, angle;
	int n, r, g, b;
	if (sscanf(parameters, "%lf %lf %lf %lf %lf %d %d %d %d ",
			   &t.x, &t.y, &steps, &t.orient, &angle, &n, &r, &g, &b) != 9) {
		return;
	}

	char *commands = generate_rule(&snap->lsys, n);

	turtle stack[1000];
	int top = 0;

	for (int i = 0; commands[i]; i++) {
		char c = commands[i];
		if (c == 'F') {
			double rad = t.orient * PI / 180.00;
			double next_x = t.x + steps * cos(rad);
			double next_y = t.y + steps * sin(rad);
			draw_line(&snap->img, round(t.x), round(t.y),
					  round(next_x), round(next_y), r, g, b);
			t.x = next_x;
			t.y = next_y;
		} else if (c == '+') {
			t.orient += angle;
		} else if (c == '-') {
			t.orient -= angle;
		} else if (c == '[') {
			if (top < 1000) {
				stack[top] = t;
				top++;
			}
		} else if (c == ']') {
			if (top > 0) {
				top--;
				t = stack[top];
			}
		}
	}

	if (!snap->silent) {
		printf("Drawing done\n");
	}
	free(commands);
}

//UNDO/REDO Case Functions

//Calls the Appropiate Function
void run_command(state *snap, char *full_line)
{
	char command[10], argument[FILE_NAME];

	//Separate full_line into 2 strings
	if (sscanf(full_line, "%s %[^\n]", command, argument) != 2) {
		return;
	}

	//Check which command to run
	if (strcmp(command, "LSYSTEM") == 0) {
		load_lsystem(snap, argument);
	} else if (strcmp(command, "LOAD") == 0) {
		load_image(snap, argument);
	} else if (strcmp(command, "TURTLE") == 0) {
		run_turtle(snap, argument);
	}
}

//Clears Current Data
void reset_state(state *snap)
{
	snap->lsys.loaded = 0;
	if (snap->img.loaded && snap->img.pixel_mat) {
		free(snap->img.pixel_mat);
		snap->img.pixel_mat = NULL;
	}
	snap->img.loaded = 0;
}

//Re-Executes All Commands Up to the Required State
void restore_state(state *snap, int is_redo)
{
	reset_state(snap);
	list *current = snap->head;
	int cnt = 0;

	while (current && cnt < snap->current_cmds) {

		if (!is_redo) {
			snap->silent = 1;
		} else {
			if (cnt == snap->current_cmds - 1) {
				snap->silent = 0;
			} else {
				snap->silent = 1;
			}
		}

		char full_line[CMD_LEN];
		strcpy(full_line, current->phrase);
		run_command(snap, full_line);
		current = current->next;
		cnt++;
	}
	snap->silent = 0;
}

//Rewrite History Function
void rewrite_list(state *snap, int index)
{
	if (!snap->head || index < 0) {
		return;
	}
	list *current = snap->head;
	list *previous = NULL;
	int count = 0;

	while (current && count < index) {
		previous = current;
		current = current->next;
		count++;
	}

	while (current) {
		list *temp = current;
		current = current->next;
		free(temp);
	}

	if (previous) {
		previous->next = NULL;
	} else {
		snap->head = NULL;
	}	
	snap->list_length = index;
}

//Adds New Command to the Linked List
void save_command(state *snap, char *command)
{
	if (snap->current_cmds < snap->list_length) {
		rewrite_list(snap, snap->current_cmds);
	}

	list *new = malloc(sizeof(list));
	strcpy(new->phrase, command);
	new->next = NULL;

	if (!snap->head) {
		snap->head = new;
	} else {
		list *temp = snap->head;
		while (temp->next) {
			temp = temp->next;
		}
		temp->next = new;
	}

	snap->list_length++;
	snap->current_cmds++;
}

//SAVE Function
void save_image(state *snap, char *img_file)
{
	//Verify if an image has been loaded
	if (!snap->img.loaded) {
		printf("No image loaded\n");
		return;
	}

	FILE *f = fopen(img_file, "wb");
	fprintf(f, "P6\n%d %d\n255\n", snap->img.width, snap->img.height);
	fwrite(snap->img.pixel_mat, sizeof(pixel),
		   snap->img.width * snap->img.height, f);
	fclose(f);
	printf("Saved %s\n", img_file);
}

int main(void)
{
	char command[10], argument[FILE_NAME], full_line[CMD_LEN];
	state snapshot = {0};
	scanf("%s", command);
	while (strcmp(command, "EXIT") != 0) {
		//LSYSTEM
		if (strcmp(command, "LSYSTEM") == 0) {
			scanf("%s", argument);
			sprintf(full_line, "LSYSTEM %s", argument);
			save_command(&snapshot, full_line);
			load_lsystem(&snapshot, argument);
		//LOAD
		} else if (strcmp(command, "LOAD") == 0) {
			scanf("%s", argument);
			sprintf(full_line, "LOAD %s", argument);
			save_command(&snapshot, full_line);
			load_image(&snapshot, argument);
		//TURTLE
		} else if (strcmp(command, "TURTLE") == 0) {
			getchar();
			fgets(argument, FILE_NAME, stdin);
			argument[strcspn(argument, "\n")] = 0;
			sprintf(full_line, "TURTLE %s", argument);
			save_command(&snapshot, full_line);
			run_turtle(&snapshot, argument);
		//UNDO
		} else if (strcmp(command, "UNDO") == 0) {
			if (snapshot.current_cmds > 0) {
				snapshot.current_cmds--;
				restore_state(&snapshot, 0);
			} else {
				printf("Nothing to undo\n");
			}
		//REDO
		} else if (strcmp(command, "REDO") == 0) {
			if (snapshot.current_cmds < snapshot.list_length) {
				snapshot.current_cmds++;
				restore_state(&snapshot, 1);
			} else {
				printf("Nothing to redo\n");
			}
		//DERIVE
		} else if (strcmp(command, "DERIVE") == 0) {
			derive(&snapshot.lsys);
		//SAVE
		} else if (strcmp(command, "SAVE") == 0) {
			scanf("%s", argument);
			save_image(&snapshot, argument);
		} else {
			break;
		}
		scanf("%s", command);
	}

	reset_state(&snapshot);
	rewrite_list(&snapshot, 0);
	return 0;
}

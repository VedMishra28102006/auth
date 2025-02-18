#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUFFER 10240
#define MAX_FILE 10240

void error(char *msg) {
	fprintf(stderr, "Error: %s\r\n", msg);
	exit(1);
}

typedef struct {
	int len;
	char **rows;
	char **cols;
} CallbackData;

static int callback(void *data, int argc, char **argv, char **azColName) {
	CallbackData *cbData = (CallbackData *)data;
	cbData->len = argc;
	cbData->rows = (char **)malloc(argc * sizeof(char *));
	cbData->cols = (char **)malloc(argc * sizeof(char *));

	for (int i = 0; i < argc; i++) {
		cbData->rows[i] = strdup(argv[i]);
		cbData->cols[i] = strdup(azColName[i]);
	}
	return 0;
}

char **extractFormData(char *buffer, char *name) {
	char *name2 = (char *)malloc(strlen(name)+1*sizeof(char));
	sprintf(name2, name);
	char key[7 + strlen(name2)];
	sprintf(key, "name=\"%s\"", name2);
	char *startpos = strstr(buffer, key);
	if (startpos == NULL) {
		free(name2);
		return NULL;
	};
	startpos = strstr(startpos, "\r\n\r\n");
	if (startpos == NULL) {
		free(name2);
		return NULL;
	}
	startpos += 4;
	char *endpos = strstr(startpos, "\r\n");
	if (endpos == NULL) endpos = buffer + strlen(buffer);
	int length = endpos - startpos;
	char *data = (char *)malloc((length + 1) * sizeof(char));
	if (data == NULL) {
		free(name2);
		free(data);
		return NULL;
	}
	strncpy(data, startpos, length);
	data[length] = '\0';
	char **temp = (char **)malloc(sizeof(char *)*2);
	temp[0] = name2;
	temp[1] = data;
	return temp;
}

int main(int argc, char **argv) {
	sqlite3 *db;
	int rc = sqlite3_open("data.db", &db);
	if (rc) error("Unable to open data.db");
	char stmt[MAX_BUFFER];
	sprintf(stmt, "CREATE TABLE IF NOT EXISTS Users ("\
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"\
		"username TEXT(50) NOT NULL,"\
		"email VARCHAR(50) NOT NULL,"\
		"password VARCHAR(255) NOT NULL"\
	")");
	rc = sqlite3_exec(db, stmt, callback, 0, 0);
	if (rc != SQLITE_OK) error("Unable to create table Users");
	if (argc < 2) error("No port has been defined");
	int portno = atoi(argv[1]);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("Unable to create socket");
	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	int binding = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (binding < 0) error("Unable to bind socket");
	int listening = listen(sockfd, 5);
	if (listening < 0) error("Unable to listen requests");
	printf("Listening for requests on port: %i\r\n", portno);
	while(1) {
		int newsockfd = accept(sockfd, NULL, NULL);
		if (newsockfd < 0) error("Unable to accept connection");
		char buffer[MAX_BUFFER];
		memset(&buffer, 0, sizeof(buffer));
		int n = read(newsockfd, buffer, sizeof(buffer)-1);
		if (n < 0) error("Unable to read request");
		buffer[n] = '\0';
		printf("Request:\r\n%s\r\n", buffer);
		char response[MAX_FILE];
		if (strncmp(buffer, "GET /style ", 11) == 0) {
			FILE *fptr = fopen("Views/style.css", "r");
			if (fptr == NULL) error("style.css file not found");
			char css[MAX_FILE];
			memset(&css, 0, sizeof(css));
			fread(css, 1, sizeof(css), fptr);
			fclose(fptr);
			sprintf(response,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/css\r\n"
				"Content-Length: %zu\r\n\r\n%s",
			strlen(css), css);
		} else if (strncmp(buffer, "GET /signup ", 12) == 0) {
			FILE *fptr = fopen("Views/signup.html", "r");
			if (fptr == NULL) error("signup.html file not found");
			char html[MAX_FILE];
			memset(&html, 0, sizeof(html));
			fread(html, 1, sizeof(html), fptr);
			fclose(fptr);
			sprintf(response,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: %zu\r\n\r\n%s",
			strlen(html), html);
		} else if (strncmp(buffer, "GET /signin ", 12) == 0) {
			FILE *fptr = fopen("Views/signin.html", "r");
			if (fptr == NULL) error("signin.html file not found");
			char html[MAX_FILE];
			memset(&html, 0, sizeof(html));
			fread(html, 1, sizeof(html), fptr);
			fclose(fptr);
			sprintf(response,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: %zu\r\n\r\n%s",
			strlen(html), html);
		} else if (strncmp(buffer, "GET /delete ", 12) == 0) {
			FILE *fptr = fopen("Views/delete.html", "r");
			if (fptr == NULL) error("delete.html file not found");
			char html[MAX_FILE];
			memset(&html, 0, sizeof(html));
			fread(html, 1, sizeof(html), fptr);
			fclose(fptr);
			sprintf(response,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: %zu\r\n\r\n%s",
			strlen(html), html);
		} else if (strncmp(buffer, "GET /forgotpassword ", 20) == 0) {
			FILE *fptr = fopen("Views/forgotpassword.html", "r");
			if (fptr == NULL) error("forgotpassword.html file not found");
			char html[MAX_FILE];
			memset(&html, 0, sizeof(html));
			fread(html, 1, sizeof(html), fptr);
			fclose(fptr);
			sprintf(response,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: %zu\r\n\r\n%s",
			strlen(html), html);
		} else if (strncmp(buffer, "POST /signup ", 13) == 0) {
			char **temp[4];
			temp[0] = extractFormData(buffer, "username");
			temp[1] = extractFormData(buffer, "email");
			temp[2] = extractFormData(buffer, "password");
			temp[3] = extractFormData(buffer, "confirmPassword");
			sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
			int reslen = strlen(response);
			if (strcmp(temp[2][1], temp[3][1]) != 0) {
				sprintf(response, 
					"HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n"
					"%s,The passwords do not match",
				temp[3][0]);
			}
			for (int i=0; i < 4; i++) {
				if (strcmp(temp[i][1], "") == 0) {
					sprintf(response, 
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n\r\n"
						"%s,Field %s is empty",
					temp[i][0], temp[i][0]);
					break;
				}
			}
			CallbackData cbData;
			sprintf(stmt, "SELECT * FROM Users");
			rc = sqlite3_exec(db, stmt, callback, &cbData, 0);
			for (int i=0; i < cbData.len; i++) {
				if (strcmp(cbData.cols[i], "username") == 0 && strcmp(cbData.rows[i], temp[0][1]) == 0) {
					sprintf(response, 
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n\r\n"
						"%s,An account with the same username already exists",
					temp[0][0]);
					break;
				} else if (strcmp(cbData.cols[i], "email") == 0 && strcmp(cbData.rows[i], temp[1][1]) == 0) {
					sprintf(response, 
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n\r\n"
						"%s,An account with the same email already exists",
					temp[1][0]);
					break;
				}
			}
			for (int i = 0; i < cbData.len; i++) {
    				free(cbData.rows[i]);
    				free(cbData.cols[i]);
			}
			free(cbData.rows);
			free(cbData.cols);

			if (strlen(response) == reslen) {
				sprintf(stmt, "INSERT INTO Users (username, email, password) VALUES ('%s', '%s', '%s')", temp[0][1], temp[1][1], temp[2][1]);
				rc = sqlite3_exec(db, stmt, callback, 0, 0);
				if (rc != SQLITE_OK) error("Unable to insert data");
			}
			for (int i=0; i < 4; i++) {
				free(temp[i][0]);
				free(temp[i][1]);
				free(temp[i]);
			}
		} else if (strncmp(buffer, "POST /signin ", 13) == 0) {
			char **temp[2];
			temp[0] = extractFormData(buffer, "username");
			temp[1] = extractFormData(buffer, "password");
			sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
			CallbackData cbData;
			sprintf(stmt, "SELECT * FROM Users");
			rc = sqlite3_exec(db, stmt, callback, &cbData, 0);
			for (int i=0; i < cbData.len; i++) {
				if (strcmp(cbData.cols[i], "username") == 0 && strcmp(cbData.rows[i], temp[0][1]) == 0) {
					if (strcmp(cbData.cols[i+2], "password") == 0 && strcmp(cbData.rows[i+2], temp[1][1]) == 0) {
						sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
					} else {
						sprintf(response, 
							"HTTP/1.1 400 Bad Request\r\n"
							"Content-Type: text/csv\r\n\r\n"
						"%s,Wrong password",
						temp[1][0]);
					}
					break;
				} else {
					sprintf(response, 
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n\r\n"
						"%s,An account with the same username does not exist",
					temp[0][0]);
				}
			}
			for (int i = 0; i < cbData.len; i++) {
    				free(cbData.rows[i]);
    				free(cbData.cols[i]);
			}
			free(cbData.rows);
			free(cbData.cols);
			for (int i=0; i < 2; i++) {
				free(temp[i][0]);
				free(temp[i][1]);
				free(temp[i]);
			}
		} else if (strncmp(buffer, "DELETE /delete ", 15) == 0) {
			char **temp[2];
			temp[0] = extractFormData(buffer, "username");
			temp[1] = extractFormData(buffer, "password");
			sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
			CallbackData cbData;
			sprintf(stmt, "SELECT * FROM Users WHERE username='%s' AND password='%s'", temp[0][1], temp[1][1]);
			rc = sqlite3_exec(db, stmt, callback, &cbData, 0);
			if (cbData.len > 0 && cbData.rows[0] != NULL) {
				sprintf(stmt, "DELETE FROM Users WHERE username='%s' AND password='%s'", temp[0][1], temp[1][1]);
				rc = sqlite3_exec(db, stmt, callback, 0, 0);
				if (rc == SQLITE_OK) {
					sprintf(response, "HTTP/1.1 200 OK\r\n\r\nUser deleted successfully.");
				} else {
					sprintf(response, "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to delete user.");
				}
			} else {
				sprintf(response,
					"HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n"
					"%s,An account with the same username and password does not exist",
				temp[0][0]);
			}
			for (int i = 0; i < cbData.len; i++) {
				free(cbData.rows[i]);
				free(cbData.cols[i]);
			}
			free(cbData.rows);
			free(cbData.cols);
			for (int i=0; i < 2; i++) {
				free(temp[i][0]);
				free(temp[i][1]);
				free(temp[i]);
			}
		} else if (strncmp(buffer, "PUT /forgotpassword ", 20) == 0) {
			char **temp[2];
			temp[0] = extractFormData(buffer, "username");
			temp[1] = extractFormData(buffer, "password");
			temp[2] = extractFormData(buffer, "confirmPassword");
			sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
			CallbackData cbData;
			sprintf(stmt, "SELECT * FROM Users WHERE username='%s'", temp[0][1]);
			rc = sqlite3_exec(db, stmt, callback, &cbData, 0);
			if (cbData.len > 0 && cbData.rows[0] != NULL) {
				if (strcmp(temp[1][1], temp[2][1]) != 0) {
					sprintf(response, 
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n\r\n"
						"%s,The passwords do not match",
					temp[2][0]);
				} else if (strcmp(temp[1][1], "") == 0) {
					sprintf(response, 
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n\r\n"
						"%s,The field password is empty",
					temp[1][0]);
				} else {
					sprintf(stmt, "UPDATE Users SET password='%s' WHERE username='%s'", temp[1][1], temp[0][1]);
					rc = sqlite3_exec(db, stmt, callback, 0, 0);
					if (rc == SQLITE_OK) {
						sprintf(response, "HTTP/1.1 200 OK\r\n\r\nUser updated successfully.");
					} else {
						sprintf(response, "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to delete user.");
					}
				}
			} else {
				sprintf(response,
					"HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n"
					"%s,An account with the same username does not exist",
				temp[0][0]);
			}
			for (int i = 0; i < cbData.len; i++) {
				free(cbData.rows[i]);
				free(cbData.cols[i]);
			}
			free(cbData.rows);
			free(cbData.cols);
			for (int i=0; i < 3; i++) {
				free(temp[i][0]);
				free(temp[i][1]);
				free(temp[i]);
			}
		} else {
			char html[MAX_FILE];
			sprintf(html, "<h1>Page not found</h1>");
			sprintf(response,
				"HTTP/1.1 404 Not Found\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: %zu\r\n\r\n%s",
			strlen(html), html);
		}
		n = write(newsockfd, response, strlen(response));
		if (n < 0) error("Unable to write response");
		close(newsockfd);
	}
	sqlite3_close(db);
	close(sockfd);
	exit(0);
}
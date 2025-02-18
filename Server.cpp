#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUFFER 10240

void error(std::string msg) {
	std::cerr << "Error: " << msg << std::endl;
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
	if (!cbData->rows) error("Unable to locate memory for rows");
	cbData->cols = (char **)malloc(argc * sizeof(char *));
	if (!cbData->cols) error("Unable to allocate memory for cols");
	for (int i = 0; i < argc; i++) {
		cbData->rows[i] = strdup(argv[i]);
		cbData->cols[i] = strdup(azColName[i]);
	}
	return 0;
}

std::vector<std::string> extractFormData(const std::string &req, const std::string &name) {
    std::string key = "name=\"" + name + "\"";
    size_t starter = req.find(key);
    if (starter == std::string::npos) {
        return {};
    }
    starter += key.length() + 4;
    std::string data;
    while (starter < req.size() && req[starter] != '\r') {
        data += req[starter];
        starter++;
    }
    return {name, data};
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
	std::cout << "Listening for requests on port: " << portno << std::endl;
	while(1) {
		int newsockfd = accept(sockfd, NULL, NULL);
		if (newsockfd < 0) error("Unable to accept connection");
		char buffer[MAX_BUFFER];
		memset(&buffer, 0, sizeof(buffer));
		int n = read(newsockfd, buffer, sizeof(buffer)-1);
		if (n < 0) error("Unable to read request");
		buffer[n] = '\0';
		std::cout << "Request:" << std::endl << buffer << std::endl;
		std::string response;
		if (strncmp(buffer, "GET /style ", 11) == 0) {
			std::ifstream fptr("Views/style.css");
			if (!fptr) error("style.css file not found");
			std::string css(
				(std::istreambuf_iterator<char>(fptr)),
				std::istreambuf_iterator<char>()
			);
			fptr.close();
			response = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/css\r\n"
			"Content-Length: " + std::to_string(css.length()) + "\r\n\r\n"
			+ css;
		} else if (strncmp(buffer, "GET /signup ", 12) == 0) {
			std::ifstream fptr("Views/signup.html");
			if (!fptr) error("signup.html file not found");
			std::string html(
				(std::istreambuf_iterator<char>(fptr)),
				std::istreambuf_iterator<char>()
			);
			fptr.close();
			response = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + std::to_string(html.length()) + "\r\n\r\n"
			+ html;
		} else if (strncmp(buffer, "GET /signin ", 12) == 0) {
			std::ifstream fptr("Views/signin.html");
			if (!fptr) error("signin.html file not found");
			std::string html(
				(std::istreambuf_iterator<char>(fptr)),
				std::istreambuf_iterator<char>()
			);
			fptr.close();
			response = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + std::to_string(html.length()) + "\r\n\r\n"
			+ html;
		} else if (strncmp(buffer, "GET /delete ", 12) == 0) {
			std::ifstream fptr("Views/delete.html");
			if (!fptr) error("delete.html file not found");
			std::string html(
				(std::istreambuf_iterator<char>(fptr)),
				std::istreambuf_iterator<char>()
			);
			fptr.close();
			response = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + std::to_string(html.length()) + "\r\n\r\n"
			+ html;
		} else if (strncmp(buffer, "GET /forgotpassword ", 20) == 0) {
			std::ifstream fptr("Views/forgotpassword.html");
			if (!fptr) error("forgotpassword.html file not found");
			std::string html(
				(std::istreambuf_iterator<char>(fptr)),
				std::istreambuf_iterator<char>()
			);
			fptr.close();
			response = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + std::to_string(html.length()) + "\r\n\r\n"
			+ html;
		} else if (strncmp(buffer, "POST /signup ", 13) == 0) {
			std::vector<std::vector<std::string>> temp(4);
			temp[0] = extractFormData(buffer, "username");
			temp[1] = extractFormData(buffer, "email");
			temp[2] = extractFormData(buffer, "password");
			temp[3] = extractFormData(buffer, "confirmPassword");
			response = "HTTP/1.1 200 OK\r\n\r\n";
			int reslen = response.length();
			if (temp[2][1] != temp[3][1]) {
				response = "HTTP/1.1 400 Bad Request\r\n"
				"Content-Type: text/csv\r\n\r\n" +
				temp[3][0] + ",The passwords do not match";
			}
			for (int i = 0; i < 4; i++) {
				if (temp[i][1].empty()) {
					response = "HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n" +
					temp[i][0] + ",Field " + temp[i][0] + " is empty";
					break;
				}
			}
			CallbackData cbData;
			sprintf(stmt, "SELECT * FROM Users");
			rc = sqlite3_exec(db, stmt, callback, &cbData, 0);
			for (int i=0; i < cbData.len; i++) {
				if (strcmp(cbData.cols[i], "username") == 0 && strcmp(cbData.rows[i], temp[0][1].c_str()) == 0) {
					response = "HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n" +
					temp[0][0] + ",An account with the same username already exists";
					break;
				} else if (strcmp(cbData.cols[i], "email") == 0 && strcmp(cbData.rows[i], temp[1][1].c_str()) == 0) {
					response = "HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n" +
					temp[1][0] + ",An account with the same email already exists";
					break;
				}
			}
			for (int i = 0; i < cbData.len; i++) {
    				free(cbData.rows[i]);
    				free(cbData.cols[i]);
			}
			free(cbData.rows);
			free(cbData.cols);

			if (response.length() == reslen) {
				sprintf(stmt, "INSERT INTO Users (username, email, password) VALUES ('%s', '%s', '%s')", temp[0][1].c_str(), temp[1][1].c_str(), temp[2][1].c_str());
				rc = sqlite3_exec(db, stmt, callback, 0, 0);
				if (rc != SQLITE_OK) error("Unable to insert data");
			}
		} else if (strncmp(buffer, "POST /signin ", 13) == 0) {
			std::vector<std::vector<std::string>> temp(4);
			temp[0] = extractFormData(buffer, "username");
			temp[1] = extractFormData(buffer, "password");
			response = "HTTP/1.1 200 OK\r\n\r\n";
			CallbackData cbData;
			sprintf(stmt, "SELECT * FROM Users");
			rc = sqlite3_exec(db, stmt, callback, &cbData, 0);
			for (int i=0; i < cbData.len; i++) {
				if (strcmp(cbData.cols[i], "username") == 0 && strcmp(cbData.rows[i], temp[0][1].c_str()) == 0) {
					if (strcmp(cbData.cols[i+2], "password") == 0 && strcmp(cbData.rows[i+2], temp[1][1].c_str()) == 0) {
						response = "HTTP/1.1 200 OK\r\n\r\n";
					} else {
						response = "HTTP/1.1 400 Bad Request\r\n"
							"Content-Type: text/csv\r\n\r\n" +
						temp[1][0] + ",Wrong password";
					}
					break;
				} else {
					response = "HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n\r\n" +
					temp[0][0] + ",An account with the same username does not exist";
				}
			}
			for (int i = 0; i < cbData.len; i++) {
    				free(cbData.rows[i]);
    				free(cbData.cols[i]);
			}
			free(cbData.rows);
			free(cbData.cols);
		} else if (strncmp(buffer, "DELETE /delete ", 15) == 0) {
			std::vector<std::vector<std::string>> temp(4);
			temp[0] = extractFormData(buffer, "username");
			temp[1] = extractFormData(buffer, "password");
			response = "HTTP/1.1 200 OK\r\n\r\n";
			CallbackData cbData;
			sprintf(stmt, "SELECT * FROM Users WHERE username='%s' AND password='%s'", temp[0][1].c_str(), temp[1][1].c_str());
			rc = sqlite3_exec(db, stmt, callback, &cbData, 0);
			if (cbData.len > 0 && cbData.rows[0] != NULL) {
				sprintf(stmt, "DELETE FROM Users WHERE username='%s' AND password='%s'", temp[0][1].c_str(), temp[1][1].c_str());
				rc = sqlite3_exec(db, stmt, callback, 0, 0);
				if (rc == SQLITE_OK) {
					response = "HTTP/1.1 200 OK\r\n\r\nUser deleted successfully.";
				} else {
					response = "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to delete user.";
				}
			} else {
				response = "HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n" +
					temp[0][0] + ",An account with the same username and password does not exist";
			}
			for (int i = 0; i < cbData.len; i++) {
				free(cbData.rows[i]);
				free(cbData.cols[i]);
			}
			free(cbData.rows);
			free(cbData.cols);
		} else if (strncmp(buffer, "PUT /forgotpassword ", 20) == 0) {
			std::vector<std::vector<std::string>> temp(4);
			temp[0] = extractFormData(buffer, "username");
			temp[1] = extractFormData(buffer, "password");
			temp[2] = extractFormData(buffer, "confirmPassword");
			response = "HTTP/1.1 200 OK\r\n\r\n";
			CallbackData cbData;
			sprintf(stmt, "SELECT * FROM Users WHERE username='%s'", temp[0][1].c_str());
			rc = sqlite3_exec(db, stmt, callback, &cbData, 0);
			if (cbData.len > 0 && cbData.rows[0] != NULL) {
				if (temp[1][1] != temp[2][1]) {
					response = "HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n" +
					temp[2][0] + ",The passwords do not match";
				} else if (temp[1][1] == "") {
					response = "HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n" +
					temp[1][0] + ",The field password is empty";
				} else {
					sprintf(stmt, "UPDATE Users SET password='%s' WHERE username='%s'", temp[1][1].c_str(), temp[0][1].c_str());
					rc = sqlite3_exec(db, stmt, callback, 0, 0);
					if (rc == SQLITE_OK) {
						response = "HTTP/1.1 200 OK\r\n\r\nUser updated successfully.";
					} else {
						response = "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to update user.";
					}
				}
			} else {
				response = "HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n\r\n" +
					temp[0][0] + ",An account with the same username and password does not exist";
			}
			for (int i = 0; i < cbData.len; i++) {
				free(cbData.rows[i]);
				free(cbData.cols[i]);
			}
			free(cbData.rows);
			free(cbData.cols);
		} else {
			std::string html = "<h1>Page not found";
			response = "HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + std::to_string(html.length()) + "\r\n\r\n"
			+ html;
		}
		n = write(newsockfd, response.c_str(), response.length());
		if (n < 0) error("Unable to write response");
		close(newsockfd);
	}
	sqlite3_close(db);
	close(sockfd);
	exit(0);
}
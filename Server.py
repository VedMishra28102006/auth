import sys
import socket
import sqlite3

MAX_BUFFER = 10240

def error(msg):
	print(msg)
	exit(1)

def extractFormData(req, name):
	key = f"name=\"{name}\""
	starter = req.find(key)
	if starter == -1:
		return ["", ""]
	starter += len(key) + 4
	data = ""
	while req[starter] != '\r' and starter < len(req):
		data += req[starter]
		starter += 1
	return [name, data]

if __name__ == "__main__":
	conn = sqlite3.connect("data.db")
	stmt = '''CREATE TABLE IF NOT EXISTS Users(
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		username TEXT(50) NOT NULL,
		email VARCHAR(50) NOT NULL,
		password VARCHAR(255) NOT NULL
	)'''
	conn.execute(stmt)
	if len(sys.argv) < 2:
		error("No port has been specified")
	portno = int(sys.argv[1])
	sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	if not sockfd:
		error("Unable to create socket")
	sockfd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	sockfd.bind(("", portno))
	sockfd.listen(5)
	print(f"Listening for requests on port: {portno}")
	while True:
		newsockfd, addr = sockfd.accept()
		if not newsockfd:
			error("Unable to accept connection")
		req = newsockfd.recv(MAX_BUFFER).decode()
		print(f"Request:\r\n{req}")
		res = ""
		if (req.startswith("GET /style ")):
			fptr = open("Views/style.css", "r")
			css = fptr.read()
			fptr.close()
			res = (
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/css\r\n"
				f"Content-Length: {len(css)}\r\n\r\n{css}"
			)
		elif (req.startswith("GET /signup ")):
			fptr = open("Views/signup.html", "r")
			html = fptr.read()
			fptr.close()
			res = (
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				f"Content-Length: {len(html)}\r\n\r\n{html}"
			)
		elif (req.startswith("GET /signin ")):
			fptr = open("Views/signin.html", "r")
			html = fptr.read()
			fptr.close()
			res = (
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				f"Content-Length: {len(html)}\r\n\r\n{html}"
			)
		elif (req.startswith("GET /delete ")):
			fptr = open("Views/delete.html", "r")
			html = fptr.read()
			fptr.close()
			res = (
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				f"Content-Length: {len(html)}\r\n\r\n{html}"
			)
		elif (req.startswith("GET /forgotpassword ")):
			fptr = open("Views/forgotpassword.html", "r")
			html = fptr.read()
			fptr.close()
			res = (
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				f"Content-Length: {len(html)}\r\n\r\n{html}"
			)
		elif (req.startswith("POST /signup")):
			fields = [
				extractFormData(req, "username"),
				extractFormData(req, "email"),
				extractFormData(req, "password"),
				extractFormData(req, "confirmPassword")
			]
			res = "HTTP/1.1 200 OK\r\n\r\n"
			resLen = len(res)
			for i in fields:
				if not i[1]:
					err = "Field " + i[0] + " is empty" 
					res = (
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n"
						f"Content-Length: {len(i[0])+len(err)+1}\r\n\r\n"
						f"{i[0]},{err}"
					)
					break
			cursor = conn.execute("SELECT * FROM Users")
			for row in cursor:
				if row[1] == fields[0][1]:
					err = "An account with the same username already exists"
					res = (
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n"
						f"Content-Length: {len(fields[0][0])+len(err)+1}\r\n\r\n"
						f"{fields[0][0]},{err}"
					)
					break
				elif row[2] == fields[1][1]:
					err = "An account with the same email already exists"
					res = (
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n"
						f"Content-Length: {len(fields[1][0])+len(err)+1}\r\n\r\n"
						f"{fields[1][0]},{err}"
					)
					break
			if (resLen == len(res)):
				if fields[2][1] != fields[3][1]:
					err = "The passwords do not match"
					res = (
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n"
						f"Content-Length: {len(fields[3][0])+len(err)+1}\r\n\r\n"
						f"{fields[3][0]},{err}"
					)
				else:
					conn.execute(f'''
						INSERT INTO Users ({fields[0][0]}, {fields[1][0]}, {fields[2][0]}) 
						VALUES ('{fields[0][1]}', '{fields[1][1]}', '{fields[2][1]}')
					''')
					conn.commit()
		elif (req.startswith("POST /signin")):
			fields = [
				extractFormData(req, "username"),
				extractFormData(req, "password")
			]
			res = "HTTP/1.1 200 OK\r\n\r\n"
			cursor = conn.execute("SELECT * FROM Users")
			for row in cursor:
				if row[1] == fields[0][1]:
					if row[3] == fields[1][1]:
						res = "HTTP/1.1 200 OK\r\n\r\n"
						break
					else:
						err = "Wrong password"
						res = (
							"HTTP/1.1 400 Bad Request\r\n"
							"Content-Type: text/csv\r\n"
							f"Content-Length: {len(fields[1][0])+len(err)+1}\r\n\r\n"
							f"{fields[1][0]},{err}"
						)
						break
				else:
					err = "Account with the username does not exist"
					res = (
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n"
						f"Content-Length: {len(fields[0][0])+len(err)+1}\r\n\r\n"
						f"{fields[0][0]},{err}"
					)
		elif (req.startswith("DELETE /delete")):
			fields = [
				extractFormData(req, "username"),
				extractFormData(req, "password")
			]
			cursor = conn.execute("SELECT * FROM Users WHERE username=? AND password=?", (fields[0][1], fields[1][1]))
			row = cursor.fetchone()

			if row:
				conn.execute("DELETE FROM Users WHERE username=? AND password=?", (fields[0][1], fields[1][1]))
				conn.commit()
				res = "HTTP/1.1 200 OK\r\n\r\n"
			else:
				err = "An account with the same username and password does not exist"
				res = (
					"HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n"
					f"Content-Length: {len(fields[0][0])+len(err)+1}\r\n\r\n"
					f"{fields[0][0]},{err}"
				)     
		elif (req.startswith("PUT /forgotpassword")):
			fields = [
				extractFormData(req, "username"),
				extractFormData(req, "password"),
				extractFormData(req, "confirmPassword")
			]
			cursor = conn.execute("SELECT * FROM Users WHERE username=?", (fields[0][1],))
			row = cursor.fetchone()

			if row:
				if fields[1][1] != fields[2][1]:
					err = "The passwords do not match"
					res = (
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n"
						f"Content-Length: {len(fields[2][0])+len(err)+1}\r\n\r\n"
						f"{fields[2][0]},{err}"
					)
				elif fields[1][1] == "":
					err = "The field password is empty"
					res = (
						"HTTP/1.1 400 Bad Request\r\n"
						"Content-Type: text/csv\r\n"
						f"Content-Length: {len(fields[1][0])+len(err)+1}\r\n\r\n"
						f"{fields[1][0]},{err}"
					)
				else:
					conn.execute("UPDATE Users SET password=? WHERE username=?", (fields[1][1], fields[0][1]))
					conn.commit()
					res = "HTTP/1.1 200 OK\r\n\r\n"
			else:
				err = "An account with the same username and password does not exist"
				res = (
					"HTTP/1.1 400 Bad Request\r\n"
					"Content-Type: text/csv\r\n"
					f"Content-Length: {len(fields[0][0])+len(err)+1}\r\n\r\n"
					f"{fields[0][0]},{err}"
				)           
		else:
			html = "<h1>Page not found</h1>"
			res = (
				"HTTP/1.1 404 Not Found\r\n"
				"Content-Type: text/html\r\n"
				f"Content-Length: {len(html)}\r\n\r\n{html}"
			)
		newsockfd.send(res.encode())
		newsockfd.close()
	conn.close()
	sockfd.close()
		
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.ServerSocket;
import java.net.SocketException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;

public class Server {
	private static final int MAX_BUFFER = 10240;
	private static final int MAX_FILE = 10240;
	private static String[] extractFormData(String req, String name)
	throws Exception
	{
		String key = "name=\"" + name + "\"";
		int starter = req.indexOf(key);
		if (starter == -1) return null;
		starter += key.length()+4;
		String data = "";
		while (req.charAt(starter) != '\r' && starter < req.length()) data += req.charAt(starter++);
		return new String[] {name, data};
	}
	private static void error(String msg)
	throws Exception
	{
		System.err.println(msg);
		System.exit(1);
	}
	public static void main(String[] args)
	throws Exception, IOException, SocketException
	{
		Class.forName("org.sqlite.JDBC");
		Connection conn = DriverManager.getConnection("jdbc:sqlite:data.db");
		Statement stmt = conn.createStatement();
		String sql = "CREATE TABLE IF NOT EXISTS Users ("
		+ "id INTEGER PRIMARY KEY AUTOINCREMENT,"
		+ "username TEXT(50) NOT NULL,"
		+ "email VARCHAR(50) NOT NULL,"
		+ "password VARCHAR(50) NOT NULL"
		+ ")";
		stmt.executeUpdate(sql);
		if (args.length < 1) error("No port has been specified");
		final int portno = Integer.parseInt(args[0]);
		ServerSocket sock = new ServerSocket(portno);
		System.out.println("Listening for requests on port: " + portno);
		boolean sockOn = true;
		while (sockOn) {
			Socket newsock = sock.accept();
			InputStream in = newsock.getInputStream();
			OutputStream out = newsock.getOutputStream();
			byte[] buffer = new byte[MAX_BUFFER];
			int n = in.read(buffer);
			if (n < 0) continue;
			String req = new String(buffer, 0, n);
			System.out.println("Request:\r\n" + req);
			String res;
			if (req.startsWith("GET /style ")) {
				File fptr = new File("Views/style.css");
				FileInputStream fin = new FileInputStream(fptr);
				byte[] fbuffer = new byte[MAX_FILE];
				int fn = fin.read(fbuffer);
				String css = new String(fbuffer, 0, fn);
				res = "HTTP/1.1 200 OK\r\n"
				+ "Content-Type: text/css\r\n"
				+ "Content-Length: " + css.length() + "\r\n\r\n"
				+ css;
			} else if (req.startsWith("GET /signup ")) {
				File fptr = new File("Views/signup.html");
				FileInputStream fin = new FileInputStream(fptr);
				byte[] fbuffer = new byte[MAX_FILE];
				int fn = fin.read(fbuffer);
				String html = new String(fbuffer, 0, fn);
				res = "HTTP/1.1 200 OK\r\n"
				+ "Content-Type: text/html\r\n"
				+ "Content-Length: " + html.length() + "\r\n\r\n"
				+ html;
			} else if (req.startsWith("GET /signin ")) {
				File fptr = new File("Views/signin.html");
				FileInputStream fin = new FileInputStream(fptr);
				byte[] fbuffer = new byte[MAX_FILE];
				int fn = fin.read(fbuffer);
				String html = new String(fbuffer, 0, fn);
				res = "HTTP/1.1 200 OK\r\n"
				+ "Content-Type: text/html\r\n"
				+ "Content-Length: " + html.length() + "\r\n\r\n"
				+ html;
			} else if (req.startsWith("GET /delete ")) {
				File fptr = new File("Views/delete.html");
				FileInputStream fin = new FileInputStream(fptr);
				byte[] fbuffer = new byte[MAX_FILE];
				int fn = fin.read(fbuffer);
				String html = new String(fbuffer, 0, fn);
				res = "HTTP/1.1 200 OK\r\n"
				+ "Content-Type: text/html\r\n"
				+ "Content-Length: " + html.length() + "\r\n\r\n"
				+ html;
			} else if (req.startsWith("GET /forgotpassword ")) {
				File fptr = new File("Views/forgotpassword.html");
				FileInputStream fin = new FileInputStream(fptr);
				byte[] fbuffer = new byte[MAX_FILE];
				int fn = fin.read(fbuffer);
				String html = new String(fbuffer, 0, fn);
				res = "HTTP/1.1 200 OK\r\n"
				+ "Content-Type: text/html\r\n"
				+ "Content-Length: " + html.length() + "\r\n\r\n"
				+ html;
			} else if (req.startsWith("POST /signup ")) {
				String[][] fields = new String[][] {
					extractFormData(req, "username"),
					extractFormData(req, "email"),
					extractFormData(req, "password"),
					extractFormData(req, "confirmPassword")
				};
				res = "HTTP/1.1 200 OK\r\n\r\n";
				int reslen = res.length();
				if (!fields[2][1].equals(fields[3][1])) {
					res = "HTTP/1.1 400 Bad Request\r\n"
					+ "Content-Type: text/csv\r\n\r\n"
					+ fields[3][0] + ",The passwords do not match";
				}
				for (int i=0; i < fields.length; i++) {
					if (fields[i][1].isEmpty()) {
						res = "HTTP/1.1 400 Bad Request\r\n"
						+ "Content-Type: text/csv\r\n\r\n"
						+ fields[i][0] + ",Field " + fields[i][0] + " is empty";
						break; 
					}
				}
				ResultSet rs = stmt.executeQuery("SELECT * FROM Users");
				while (rs.next()) {
					if (rs.getString("username").equals(fields[0][1])) {
						res = "HTTP/1.1 400 Bad Request\r\n"
						+ "Content-Type: text/csv\r\n\r\n"
						+ fields[0][0] + ",An account with the same username already exists";
						break; 
					} else if (rs.getString("email").equals(fields[1][1])) {
						res = "HTTP/1.1 400 Bad Request\r\n"
						+ "Content-Type: text/csv\r\n\r\n"
						+ fields[1][0] + ",An account with the same email already exists";
						break; 
					}
				}
				rs.close();
				if (reslen == res.length()) {
					sql = "INSERT INTO Users (username, email, password) VALUES ('"+fields[0][1]+"','"+fields[1][1]+"','"+fields[2][1]+"')";
					stmt.executeUpdate(sql);
				}
			} else if (req.startsWith("POST /signin ")) {
				String[][] fields = new String[][] {
					extractFormData(req, "username"),
					extractFormData(req, "password")
				};
				ResultSet rs = stmt.executeQuery("SELECT * FROM Users");
				res = "HTTP/1.1 200 OK\r\n\r\n";
				while (rs.next()) {
					if (rs.getString("username").equals(fields[0][1])) {
						if (rs.getString("password").equals(fields[1][1])) {
							res = "HTTP/1.1 200 OK\r\n\r\n";
							break;
						} else {
							res = "HTTP/1.1 400 Bad Request\r\n"
							+ "Content-Type: text/csv\r\n\r\n"
							+ fields[1][0] + ",Wrong password";
							break; 
						}
					} else  {
						res = "HTTP/1.1 400 Bad Request\r\n"
						+ "Content-Type: text/csv\r\n\r\n"
						+ fields[0][0] + ",An account with the same username does not exist";
					}
				}
				rs.close();
			} else if (req.startsWith("DELETE /delete ")) {
				String[][] fields = new String[][] {
					extractFormData(req, "username"),
					extractFormData(req, "password")
				};
				String selectQuery = "SELECT * FROM Users WHERE username='" + fields[0][1] + "' AND password='" + fields[1][1] + "' LIMIT 1";
				ResultSet rs = stmt.executeQuery(selectQuery);
				res = "HTTP/1.1 200 OK\r\n\r\n";
				if (rs.next()) {
					sql = "DELETE FROM Users WHERE username='" + fields[0][1] + "' AND password='" + fields[1][1] + "'";
					stmt.executeUpdate(sql);
					res = "HTTP/1.1 200 OK\r\n\r\n";
				} else {
					res = "HTTP/1.1 400 Bad Request\r\n"
					+ "Content-Type: text/csv\r\n\r\n"
					+ fields[0][0] + ",An account with the same username and password does not exist";
				}
				rs.close();
			} else if (req.startsWith("PUT /forgotpassword ")) {
				String[][] fields = new String[][] {
					extractFormData(req, "username"),
					extractFormData(req, "password"),
					extractFormData(req, "confirmPassword")
				};
				String selectQuery = "SELECT * FROM Users WHERE username='" + fields[0][1] + "' LIMIT 1";
				ResultSet rs = stmt.executeQuery(selectQuery);
				res = "HTTP/1.1 200 OK\r\n\r\n";
				if (rs.next()) {
					if (!fields[1][1].equals(fields[2][1])) {
						res = "HTTP/1.1 400 Bad Request\r\n"
						+ "Content-Type: text/csv\r\n\r\n"
						+ fields[2][0] + ",The passwords do not match";
					} else if (fields[1][1] == "") {
						res = "HTTP/1.1 400 Bad Request\r\n"
						+ "Content-Type: text/csv\r\n\r\n"
						+ fields[1][0] + ",The field password is empty";
					} else {
						sql = "UPDATE Users SET password ='" + fields[1][1] + "' WHERE username='" + fields[0][1] + "'";
						stmt.executeUpdate(sql);
						res = "HTTP/1.1 200 OK\r\n\r\n";
					}
				} else {
					res = "HTTP/1.1 400 Bad Request\r\n"
					+ "Content-Type: text/csv\r\n\r\n"
					+ fields[0][0] + ",An account with the same username does not exist";
				}
				rs.close();
			} else {
				String html = "<h1>Page not found</h1>";
				res = "HTTP/1.1 404 Not Found\r\n"
				+ "Content-Type: text/html\r\n"
				+ "Content-Length: " + html.length() + "\r\n\r\n"
				+ html;
			}
			out.write(res.getBytes());
			out.close();
			in.close();
			newsock.close();
		}
		stmt.close();
		conn.close();
		sock.close();
	}
}
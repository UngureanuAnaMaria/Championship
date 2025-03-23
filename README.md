# Championship - Client/Server Application for Managing Tournaments 🏆

## Description 🎮

Welcome to **Championship**, a client/server system built to streamline the management of gaming tournaments! This application allows both regular users and administrators to manage, register, and participate in various tournaments. Whether you’re creating a tournament, signing up for one, or rescheduling matches, **Championship** makes the process easy and efficient.

The server is built with **C**, and the data is stored in **SQLite** for simplicity and performance. Communication between the client and server is handled via **TCP/IP**, ensuring reliable and fast connections. The system supports multithreading, so you can expect smooth operations even with multiple users connected simultaneously.

---

## Key Features 🌟

### 1. **User Registration & Authentication 🔐**
- **User Types**: The system supports both regular users and administrators. 
- **Login Required**: You must log in to access tournament info and perform actions.
- **Email Notifications 📧**: Receive email updates when you’re accepted into a tournament, along with details about your upcoming matches (time, opponent, etc.).

### 2. **Tournament Management 🏆**
- **Create Tournaments (Admin Only)**: Administrators can create tournaments by defining the game, number of players, and tournament structure (e.g., single-elimination, double-elimination).
- **Match Scheduling ⚔️**: The system schedules matches and allows users to reschedule games when necessary.
- **Tournament History 📜**: Admins can view the history of past tournaments, including scores and winners.

### 3. **User Interaction 🤝**
- **Tournament Sign-Up**: Regular users can sign up for tournaments. If you’re accepted, you’ll get an email with match details.
- **Reschedule Matches 🔄**: If needed, users can request to reschedule their matches with the opponent’s consent.
- **Help Command 💬**: Type `help` to see a list of all available commands and their usage.

---

## Technologies Used 🛠️

### **Backend** ⚙️
- **C Language**: Used for both the server and client applications, providing a lightweight and fast solution to manage user requests.
- **SQLite**: A simple yet powerful database for storing user data, tournament details, and match histories.

### **Communication** 🌐
- **TCP/IP**: The communication between the client and server is done through TCP/IP, ensuring reliable and fast data exchange.
- **Multithreading**: The server uses multithreading to handle multiple client connections simultaneously, providing an efficient experience for users.

---

## How to Use 🖥️

1. **Start the Server**: Run the server on a desired port. The server will wait for client connections.
2. **Connect the Client**: Launch the client application and connect to the server using the server's IP address and port.
3. **Log In or Register**: Use the `login` or `register` commands to access the system.
4. **Execute Commands**: After logging in, execute commands to view tournaments, sign up, reschedule matches, or view tournament history.

---

## Conclusion 🎉

The **Championship** application provides a solid, efficient solution for managing gaming tournaments. With its easy-to-use commands, secure login system, and reliable communication, it ensures a smooth experience for both regular users and administrators. The use of **C** and **SQLite** ensures excellent performance and scalability, making it a great tool for managing a variety of tournaments.

For further customization or improvements, feel free to explore the source code and documentation. Happy gaming! 🎮

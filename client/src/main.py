import customtkinter as ctk
from tkinter import filedialog, messagebox
import socket
import threading
import time

HOST = "127.0.1.1"
PORT = 8080
PASSWORD = "abcd"

class QuizApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Kahoot")
        self.geometry("600x400")
        self.username = None
        self.socket = None
        self.lobby_id = None
        self.show_frame(WelcomeScreen)

    def show_frame(self, frame_class):
        if hasattr(self, "current_frame"):
            self.current_frame.pack_forget()
        frame = frame_class(self)
        frame.pack(fill="both", expand=True)
        self.current_frame = frame
        return frame

    def set_username(self, username):
        self.username = username

    def set_socket(self, socket):
        self.socket = socket


class WelcomeScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        ctk.CTkLabel(self, text="Welcome!", font=("Arial", 24)).pack(pady=20)
        ctk.CTkButton(self, text="Create lobby", command=lambda: parent.show_frame(CreateLobbyScreen)).pack(pady=10)
        ctk.CTkButton(self, text="Join lobby", command=lambda: parent.show_frame(JoinLobbyScreen)).pack(pady=10)
        ctk.CTkButton(self, text="Exit", command=parent.destroy).pack(pady=10)

class JoinLobbyScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        ctk.CTkLabel(self, text="Join lobby", font=("Arial", 18)).pack(pady=10)
        ctk.CTkLabel(self, text="Enter lobby ID:").pack(pady=5)
        self.lobby_id_entry = ctk.CTkEntry(self, placeholder_text="Lobby ID")
        self.lobby_id_entry.pack(pady=5)
        ctk.CTkButton(self, text="Join", command=self.join_lobby).pack(pady=10)
        ctk.CTkButton(self, text="Go back", command=lambda: parent.show_frame(WelcomeScreen)).pack(pady=10)

    def join_lobby(self):
        lobby_id = int(self.lobby_id_entry.get())
        if lobby_id:
            print(f"Joining {HOST} {lobby_id} {PASSWORD} JOIN_LOBBY {lobby_id}")
            self.master.show_frame(NicknameScreen)
            threading.Thread(
                target=connect_to_server,
                args=(HOST, lobby_id, PASSWORD, "JOIN_LOBBY", lobby_id, self),
                daemon=True
            ).start()

class NicknameScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        ctk.CTkLabel(self, text="Enter your name", font=("Arial", 24)).pack(pady=20)
        self.username_entry = ctk.CTkEntry(self, placeholder_text="Name")
        self.username_entry.pack(pady=10)
        ctk.CTkButton(self, text="Confirm", command=self.submit_username).pack(pady=10)

    def submit_username(self):
        username = self.username_entry.get()
        if username:
            self.master.set_username(username)
            print(f"User {username} joined.")
            self.master.show_frame(QuestionScreen)

class CreateLobbyScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        ctk.CTkLabel(self, text="Creating a new lobby...", font=("Arial", 18)).pack(pady=10)
        self.create_lobby()

    def create_lobby(self):
        threading.Thread(
            target=connect_to_server,
            args=(HOST, PORT, PASSWORD, "CREATE_LOBBY", None, self),
            daemon=True
        ).start()

class QuestionScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        self.current_question = None
        self.answer_buttons = []
        threading.Thread(target=self.receive_questions, daemon=True).start()

    def display_question(self, question_data):
        if hasattr(self, 'question_label'):
            self.question_label.destroy()
        for button in self.answer_buttons:
            button.destroy()

        self.question_label = ctk.CTkLabel(self, text=question_data["question"], font=("Arial", 18))
        self.question_label.pack(pady=10)

        self.answer_buttons = []
        for i, answer in enumerate(question_data["answers"]):
            button = ctk.CTkButton(self, text=f"{answer}", command=lambda ans=answer: self.submit_answer(ans))
            button.pack(pady=5)
            self.answer_buttons.append(button)

    def receive_questions(self):
        try:
            while True:
                if self.master.socket:
                    question_data = self.master.socket.recv(1024).decode()
                    if question_data.startswith("QUESTION:"):
                        parts = question_data.split(":")
                        question_text = parts[1]
                        answers = parts[2].split(",")
                        question_data = {"question": question_text, "answers": answers}
                        self.master.after(0, self.display_question, question_data)
        except Exception as e:
            print(f"Error receiving questions: {e}")

    def submit_answer(self, answer):
        try:
            if self.master.socket:
                self.master.socket.sendall(f"ANSWER:{answer}".encode())
                print(f"Answer submitted: {answer}")
        except Exception as e:
            print(f"Error submitting answer: {e}")
            
class LobbyHostScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        ctk.CTkLabel(self, text=f"Admin Lobby (ID: {parent.lobby_id})", font=("Arial", 24)).pack(pady=20)
        self.user_count_label = ctk.CTkLabel(self, text="Users Connected: 0", font=("Arial", 14))
        self.user_count_label.pack(pady=10)
        self.answer_count_label = ctk.CTkLabel(self, text="Answers Submitted: 0", font=("Arial", 14))
        self.answer_count_label.pack(pady=10)
        ctk.CTkButton(self, text="Start Quiz", command=self.start_quiz).pack(pady=20)
        ctk.CTkButton(self, text="Exit Lobby", command=lambda: parent.show_frame(WelcomeScreen)).pack(pady=10)
        threading.Thread(target=self.receive_lobby_updates, daemon=True).start()

    def receive_lobby_updates(self):
        try:
            while True:
                if self.master.socket:
                    self.master.socket.sendall(f"HOST_UPDATE:{self.master.lobby_id}".encode())
                    update = self.master.socket.recv(1024).decode()

                    if update and "CURRENT_PLAYERS" in update:
                        current_players = int(update.split(":")[2].split()[0])
                        self.master.after(0, self.update_dashboard, current_players)
                    time.sleep(5)
        except Exception as e:
            print(f"Error in lobby host updates: {e}")

    def update_dashboard(self, user_count):
        self.user_count_label.configure(text=f"Users Connected: {user_count}")

    def start_quiz(self):
        print("Starting the quiz!")

def connect_to_server(host, port, password, action, lobby_param=None, frame=None):
    print(f"Connecting to {host}:{port}")
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        client_socket.connect((host, port)) 

        if action == "CREATE_LOBBY":
            client_socket.sendall(password.encode())  
            response = client_socket.recv(1024).decode()
            print(f"Auth response: {response}")
            
            if "AUTH_SUCCESS" in response:
                client_socket.sendall("CREATE_LOBBY".encode()) 
                response = client_socket.recv(1024).decode() 

                print(f"Lobby creation response: {response}")

                if response.startswith("LOBBY_CREATED"):
                    lobby_port = int(response.split("ID:")[1].strip()) 
                    print(f"Lobby created on port {lobby_port}")

                    client_socket.close()
                    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    client_socket.connect((host, lobby_port))  
                    frame.master.set_socket(client_socket) 
                    frame.master.lobby_id = lobby_port  

                    client_socket.sendall("A JOIN_LOBBY".encode())
                    frame.master.after(0, frame.master.show_frame, LobbyHostScreen)
                else:
                    print("Lobby creation failed.")
                    messagebox.showerror("Error", "Failed to create lobby.")
            else:
                print(f"Authentication failed: {response}")
                messagebox.showerror("Error", "Authentication failed. Incorrect password.")
        
        elif action == "JOIN_LOBBY":
            client_socket.sendall("P JOIN_LOBBY".encode())  
            print(f"Successfully joined lobby {lobby_param}")
            frame.master.set_socket(client_socket)
            frame.master.after(0, frame.master.show_frame, NicknameScreen)
    
    except Exception as e:
        print(f"Error: {e}")
        messagebox.showerror("Error", str(e))
    finally:
        return client_socket


if __name__ == "__main__":
    app = QuizApp()
    app.mainloop()

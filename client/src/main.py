import customtkinter as ctk
from tkinter import filedialog, messagebox

import socket
import threading
import time

HOST = "127.0.1.1"
PORT = 8081
PASSWORD = "abcd"
CLIENT_ID = 1

class QuizApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Kahoot")
        self.geometry("600x400")
        self.username = None
        self.socket = None
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

class CreateLobbyScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        ctk.CTkLabel(self, text="Create lobby", font=("Arial", 18)).pack(pady=10)
        ctk.CTkLabel(self, text="Enter lobby name:").pack(pady=5)
        self.lobby_name_entry = ctk.CTkEntry(self, placeholder_text="Lobby name")
        self.lobby_name_entry.pack(pady=5)
        ctk.CTkButton(self, text="Create", command=self.create_lobby).pack(pady=10)
        ctk.CTkButton(self, text="Go back", command=lambda: parent.show_frame(WelcomeScreen)).pack(pady=10)

    def create_lobby(self):
        lobby_name = self.lobby_name_entry.get()
        if lobby_name:
            print(f"Creating lobby '{lobby_name}'...")
            self.master.lobby_name = lobby_name
            threading.Thread(
                target=connect_to_server,
                args=(HOST, PORT, PASSWORD, "CREATE_LOBBY", lobby_name, self),
                daemon=True
            ).start()

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
        lobby_id = self.lobby_id_entry.get()
        if lobby_id:
            print(f"Joining lobby ID {lobby_id}")
            self.master.show_frame(NicknameScreen)
            threading.Thread(
                target=connect_to_server,
                args=(HOST, PORT, PASSWORD, "JOIN_LOBBY", lobby_id, self),
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

class LobbyHostScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        self.parent = parent
        self.lobby_name = self.parent.lobby_name
        ctk.CTkLabel(self, text=f"Lobby: {self.lobby_name}", font=("Arial", 24)).pack(pady=20)
        self.user_count_label = ctk.CTkLabel(self, text="Users Connected: 0", font=("Arial", 14))
        self.user_count_label.pack(pady=10)
        self.answer_count_label = ctk.CTkLabel(self, text="Answers Submitted: 0", font=("Arial", 14))
        self.answer_count_label.pack(pady=10)
        ctk.CTkButton(self, text="Start Quiz", command=self.start_quiz).pack(pady=20)
        ctk.CTkButton(self, text="Exit Lobby", command=lambda: parent.show_frame(WelcomeScreen)).pack(pady=10)
        self.load_questions_button = ctk.CTkButton(
            self, 
            text="Load Questions", 
            command=self.load_questions
        )
        self.load_questions_button.pack(pady=10)
        self.user_count = 0
        self.answer_count = 0

        threading.Thread(target=self.receive_lobby_updates, daemon=True).start()

    def update_dashboard(self, user_count, answer_count):
        self.user_count = user_count
        self.answer_count = answer_count
        self.user_count_label.configure(text=f"Users Connected: {self.user_count}")
        self.answer_count_label.configure(text=f"Answers Submitted: {self.answer_count}")

    def receive_lobby_updates(self):
        try:
            while True:
                if self.master.socket:
                    self.master.socket.sendall(f"HOST_UPDATE:{self.master.lobby_id}".encode())
                    update = self.master.socket.recv(1024).decode()

                    if update:
                        if "CURRENT_PLAYERS" in update and "MAX_PLAYERS" in update:
                            try:
                                print(f"GUI Received update: {update}")
                                current_players = int(update.split(":")[2].split()[0])
                                max_players = int(update.split(":")[3].split()[0])
                                self.master.after(0, self.update_dashboard, current_players, max_players)
                            except ValueError:
                                print(f"Error parsing update: {update}")

                    time.sleep(5)

        except Exception as e:
            print(f"Error in lobby host updates: {e}")

    def start_quiz(self):
            print(f"Starting the quiz for lobby: {self.lobby_name}")
            if self.master.socket:
                try:
                    self.master.socket.sendall(f"SEND_QUESTION:{self.master.lobby_id}".encode())
                    print("Quiz started, question sent to the lobby.")
                except Exception as e:
                    print(f"Error starting quiz: {e}")
            self.master.show_frame(QuestionScreen)
            
    def load_questions(self):
        file_path = filedialog.askopenfilename(
            filetypes=[("Text Files", "*.txt")]
        )
        if file_path:
            try:
                with open(file_path, 'r') as file:
                    lines = file.readlines()
                    i = 0
                    while i < len(lines):
                        if len(lines[i].strip()) == 0:
                            i += 1
                            continue
                            
                        question = lines[i].strip()
                        answers = lines[i+1].strip().split('|')
                        correct_answer = int(lines[i+2].strip())
                        
                        # Format: LOAD_QUESTION;<lobby_id>;<question>;<ans1>|<ans2>|<ans3>|<ans4>;<correct>
                        message = f"LOAD_QUESTION;{self.master.lobby_id};{question};{lines[i+1].strip()};{correct_answer}"
                        self.master.socket.sendall(message.encode())
                        response = self.master.socket.recv(1024).decode()
                        
                        if "ERROR" in response:
                            messagebox.showerror("Error", f"Failed to load question: {question}")
                            return
                            
                        i += 3
                    
                messagebox.showinfo("Success", "Questions loaded successfully!")
                
            except Exception as e:
                messagebox.showerror("Error", f"Failed to load questions: {str(e)}")

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

def connect_to_server(host, port, password, action, lobby_param=None, frame=None):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((host, port))
        client_socket.sendall(password.encode())
        response = client_socket.recv(1024).decode()

        if "AUTH_SUCCESS" in response:
            if action == "CREATE_LOBBY":
                client_socket.sendall(f"CREATE_LOBBY:{lobby_param}".encode())
                response = client_socket.recv(1024).decode()
                if response.startswith("CREATE_SUCCESS"):
                    lobby_id = int(response.split("ID:")[1].split()[0])
                    frame.master.lobby_id = lobby_id
                    print(f"Lobby '{lobby_param}' created with ID {lobby_id}!")
                    frame.master.set_socket(client_socket)
                    frame.master.after(0, frame.master.show_frame, LobbyHostScreen)
                else:
                    print("Lobby creation failed.")

            elif action == "JOIN_LOBBY" and lobby_param:
                client_socket.sendall(f"JOIN_LOBBY:{lobby_param}".encode())
                response = client_socket.recv(1024).decode()
                if "JOIN_SUCCESS" in response:
                    print(f"Successfully joined lobby {lobby_param}")
                    frame.master.set_socket(client_socket)
                    frame.master.after(0, frame.master.show_frame, NicknameScreen)
                else:
                    print(f"Failed to join lobby {lobby_param}")

        else:
            print(f"Authentication failed: {response}")

    except Exception as e:
        print(f"Error: {e}")
    finally:
        return client_socket

if __name__ == "__main__":
    app = QuizApp()
    app.mainloop()

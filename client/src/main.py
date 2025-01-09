import customtkinter as ctk
import socket
import threading

HOST = "127.0.1.1"
PORT = 8080
PASSWORD = "abcd"
CLIENT_ID = 1

class QuizApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Kahoot")
        self.geometry("600x400")
        self.username = None
        self.correct_answers = 0 
        self.show_frame(WelcomeScreen)

    def show_frame(self, frame_class):
        for widget in self.winfo_children():
            widget.destroy()
        frame = frame_class(self)
        frame.pack(fill="both", expand=True)
        return frame

    def set_username(self, username):
        self.username = username

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
        self.lobby_name = ctk.CTkEntry(self, placeholder_text="Lobby name")
        self.lobby_name.pack(pady=5)
        ctk.CTkButton(self, text="Create", command=self.create_lobby).pack(pady=10)
        ctk.CTkButton(self, text="Go back", command=lambda: parent.show_frame(WelcomeScreen)).pack(pady=10)

    def create_lobby(self):
        lobby_name = self.lobby_name.get()
        if lobby_name:
            print(f"Lobby '{lobby_name}' was created!")
            self.master.show_frame(NicknameScreen)
            threading.Thread(target=connect_to_server, args=(HOST, PORT, PASSWORD, CLIENT_ID)).start()

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
            print(f"Joined lobby ID {lobby_id}")
            self.master.show_frame(NicknameScreen)

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

class QuestionScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        self.questions = [
            {
                "question": "Aká je farba neba?",
                "answers": ["Modrá", "Zelená", "Červená", "Žltá"],
                "correct_answer": "Modrá"
            },
            {
                "question": "Ktorý je hlavné mesto Francúzska?",
                "answers": ["Londýn", "Paríž", "Berlin", "Rím"],
                "correct_answer": "Paríž"
            }
        ]
        self.current_question = 0
        self.display_question()

    def display_question(self):
        question_data = self.questions[self.current_question]

        if hasattr(self, 'question_label'):
            self.question_label.destroy()
        
        if hasattr(self, 'user_info_label'):
            self.user_info_label.destroy()

        self.question_label = ctk.CTkLabel(self, text=question_data["question"], font=("Arial", 18))
        self.question_label.pack(pady=10)

        self.user_info_label = ctk.CTkLabel(self, text=f"Name: {self.master.username}  |  Score: {self.master.correct_answers}", font=("Arial", 14))
        self.user_info_label.pack(pady=10)

        self.answer_buttons = []

        for i, answer in enumerate(question_data["answers"]):
            button = ctk.CTkButton(self, text=f"{answer}", command=lambda ans=answer: self.check_answer(ans))
            button.pack(pady=5)
            self.answer_buttons.append(button)

    def check_answer(self, answer):
        correct_answer = self.questions[self.current_question]["correct_answer"]
        if answer == correct_answer:
            print("Correct answer!")
            self.master.correct_answers += 1  
        else:
            print("Incorrect answer!")
        
        self.current_question += 1
        if self.current_question < len(self.questions):
            for button in self.answer_buttons:
                button.destroy()
            self.display_question()
        else:
            print("All questions were answered.")
            self.master.correct_answers = 0
            self.master.username = None
            self.master.show_frame(WelcomeScreen)

def connect_to_server(host, port, password, client_id):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        client_socket.connect((host, port))
        print(f"Client {client_id} connected to server")

        client_socket.sendall(password.encode())
        print(f"Client {client_id}: Sent password: {password}")

        response = client_socket.recv(1024).decode()
        print(f"Client {client_id}: Server Response: {response}")

        if "AUTH_SUCCESS" in response:
            client_request = "CREATE_LOBBY"
            client_socket.sendall(client_request.encode())
            response = client_socket.recv(1024).decode()
            print(f"Client {client_id}: Server Response: {response}")

            client_request = "GET_STATUS"
            client_socket.sendall(client_request.encode())
            response = client_socket.recv(1024).decode()
            print(f"Client {client_id}: Server Response: {response}")

    except Exception as e:
        print(f"Client {client_id}: Error: {e}")
    finally:
        client_socket.close()

if __name__ == "__main__":
    app = QuizApp()
    app.mainloop()
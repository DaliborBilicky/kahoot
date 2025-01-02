import customtkinter as ctk

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
        ctk.CTkLabel(self, text="Vitajte!", font=("Arial", 24)).pack(pady=20)
        ctk.CTkButton(self, text="Vytvoriť lobby", command=lambda: parent.show_frame(CreateLobbyScreen)).pack(pady=10)
        ctk.CTkButton(self, text="Pripojiť sa do lobby", command=lambda: parent.show_frame(JoinLobbyScreen)).pack(pady=10)
        ctk.CTkButton(self, text="Opustiť", command=parent.destroy).pack(pady=10)

class CreateLobbyScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        ctk.CTkLabel(self, text="Vytvorenie lobby", font=("Arial", 18)).pack(pady=10)
        ctk.CTkLabel(self, text="Zadajte názov lobby:").pack(pady=5)
        self.lobby_name = ctk.CTkEntry(self, placeholder_text="Názov lobby")
        self.lobby_name.pack(pady=5)
        ctk.CTkButton(self, text="Vytvoriť", command=self.create_lobby).pack(pady=10)
        ctk.CTkButton(self, text="Späť", command=lambda: parent.show_frame(WelcomeScreen)).pack(pady=10)

    def create_lobby(self):
        lobby_name = self.lobby_name.get()
        if lobby_name:
            print(f"Lobby '{lobby_name}' bolo vytvorené!")
            self.master.show_frame(NicknameScreen)

class JoinLobbyScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        ctk.CTkLabel(self, text="Pripojenie k lobby", font=("Arial", 18)).pack(pady=10)
        ctk.CTkLabel(self, text="Zadajte ID lobby:").pack(pady=5)
        self.lobby_id_entry = ctk.CTkEntry(self, placeholder_text="ID lobby")
        self.lobby_id_entry.pack(pady=5)
        ctk.CTkButton(self, text="Pripojiť sa", command=self.join_lobby).pack(pady=10)
        ctk.CTkButton(self, text="Späť", command=lambda: parent.show_frame(WelcomeScreen)).pack(pady=10)

    def join_lobby(self):
        lobby_id = self.lobby_id_entry.get()
        if lobby_id:
            print(f"Pripojenie sa k lobby s ID: {lobby_id}")
            self.master.show_frame(NicknameScreen)

class NicknameScreen(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)
        ctk.CTkLabel(self, text="Zadajte svoje meno", font=("Arial", 24)).pack(pady=20)
        self.username_entry = ctk.CTkEntry(self, placeholder_text="Zadajte svoje meno")
        self.username_entry.pack(pady=10)
        ctk.CTkButton(self, text="Potvrdiť meno", command=self.submit_username).pack(pady=10)

    def submit_username(self):
        username = self.username_entry.get()
        if username:
            self.master.set_username(username)
            print(f"Užívateľ {username} bol prihlásený.")
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

        self.user_info_label = ctk.CTkLabel(self, text=f"Meno: {self.master.username}  |  Správne odpovede: {self.master.correct_answers}", font=("Arial", 14))
        self.user_info_label.pack(pady=10)

        self.answer_buttons = []
        options = ['A', 'B', 'C', 'D']
        
        for i, answer in enumerate(question_data["answers"]):
            button = ctk.CTkButton(self, text=f"{answer}", command=lambda ans=answer: self.check_answer(ans))
            button.pack(pady=5)
            self.answer_buttons.append(button)

    def check_answer(self, answer):
        correct_answer = self.questions[self.current_question]["correct_answer"]
        if answer == correct_answer:
            print("Správna odpoveď!")
            self.master.correct_answers += 1  
        else:
            print("Nesprávna odpoveď!")
        
        self.current_question += 1
        if self.current_question < len(self.questions):
            for button in self.answer_buttons:
                button.destroy()
            self.display_question()
        else:
            print("Koniec otázok.")
            self.master.correct_answers = 0
            #pozriet pri doplneni komunikaciee s vlaknami !!!!!
            self.master.username = None
            self.master.show_frame(WelcomeScreen)

if __name__ == "__main__":
    app = QuizApp()
    app.mainloop()

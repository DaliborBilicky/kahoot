import customtkinter as ctk

class QuizApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Kahoot")
        self.geometry("600x400")
        self.welcome_screen = WelcomeScreen(self)
        self.welcome_screen.pack(fill="both", expand=True)

    def show_frame(self, frame_class):
        for widget in self.winfo_children():
            widget.destroy()

        frame = frame_class(self)
        frame.pack(fill="both", expand=True)
        return frame

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
        print(f"Lobby '{self.lobby_name.get()}' bolo vytvorené!")

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
        print(f"Pripojenie sa k lobby s ID: {self.lobby_id_entry.get()}")

if __name__ == "__main__":
    app = QuizApp()
    app.mainloop()

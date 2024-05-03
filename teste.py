import tkinter as tk
from tkinter import messagebox

# Funções para responder aos botões
def registrar_usuario():
    # Aqui você pode adicionar a lógica para registrar um usuário
    messagebox.showinfo("Registrar-se", "Usuário registrado com sucesso!")

def entrar_na_sala():
    # Lógica para entrar em uma sala com um código
    sala_codigo = sala_codigo_entry.get()  # Pegue o código da sala do campo de entrada
    messagebox.showinfo("Entrar na Sala", f"Entrou na sala com código: {sala_codigo}")

def enviar_mensagem():
    # Lógica para enviar mensagens para uma sala
    mensagem = mensagem_entry.get()  # Pegue a mensagem do campo de entrada
    if mensagem:
        messagebox.showinfo("Enviar Mensagem", f"Mensagem enviada: {mensagem}")
        mensagem_entry.delete(0, tk.END)  # Limpa o campo de entrada
    else:
        messagebox.showwarning("Enviar Mensagem", "Mensagem não pode ser vazia!")

# Criação da janela principal
root = tk.Tk()
root.title("Interface de Cliente")

# Criar os botões e campos de entrada
btn_registrar = tk.Button(root, text="Registrar-se", command=registrar_usuario)
btn_entrar_sala = tk.Button(root, text="Entrar na Sala", command=entrar_na_sala)

# Campo para inserir o código da sala
sala_codigo_label = tk.Label(root, text="Código da Sala:")
sala_codigo_entry = tk.Entry(root)

# Campo para enviar uma mensagem para a sala
mensagem_label = tk.Label(root, text="Mensagem:")
mensagem_entry = tk.Entry(root)
btn_enviar_mensagem = tk.Button(root, text="Enviar Mensagem", command=enviar_mensagem)

# Posicionamento dos elementos na janela
btn_registrar.pack(pady=5)  # Adiciona espaço entre os elementos
sala_codigo_label.pack()
sala_codigo_entry.pack()
btn_entrar_sala.pack(pady=5)

mensagem_label.pack()
mensagem_entry.pack()
btn_enviar_mensagem.pack(pady=5)

# Iniciar o loop principal do Tkinter para manter a janela aberta
root.mainloop()

import tkinter as tk
from tkinter import filedialog, messagebox
import csv
from collections import OrderedDict
import os

class TranslationEditor:
    def __init__(self, root):
        root.title("Translation Editor")
        root.state('zoomed')  # Maximizes the window
        # Dropdown menu for language selection
        self.language_var = tk.StringVar(value="Select Language")
        self.language_menu = tk.OptionMenu(root, self.language_var, "en", "fr", "es", command=self.load_lang)
        self.language_menu.pack(pady=5)

        # ...création des boutons de chargement et save...
        top = tk.Frame(root); top.pack(fill='x', padx=5, pady=5)
        tk.Button(top, text="Save Lang CSV",  command=self.save_lang).pack(side='right', padx=5)
        self.missing_label = tk.Label(root, text="", fg='red'); self.missing_label.pack(fill='x', padx=5)

        # ...zone scrollable pour les champs...
        self.canvas = tk.Canvas(root); self.scrollbar = tk.Scrollbar(root, orient='vertical', command=self.canvas.yview)
        self.scrollable = tk.Frame(self.canvas)
        self.scrollable.bind("<Configure>", lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all")))
        self.canvas.create_window((0,0), window=self.scrollable, anchor='nw')
        self.canvas.configure(yscrollcommand=self.scrollbar.set)
        self.canvas.pack(side='left', fill='both', expand=True); self.scrollbar.pack(side='right', fill='y')

        self.en_data = OrderedDict(); self.lang_data = {}; self.entries = {}
        self.en_file = None;    self.lang_file = None
        
        

        self.load_en()

    def load_en(self):
        path = os.path.join(os.path.dirname(__file__), "en.csv")  # Relative to the .py file
        if not path: return
        self.lang_file = path; self.lang_data.clear()
        with open(path, encoding='utf-8', newline='') as f:
            for row in csv.reader(f, delimiter=':'):  # Utilisation de ':' comme délimiteur
                if len(row) == 2:  # Vérifie que la ligne contient exactement 2 colonnes
                    key, val = row
                    self.en_data[key] = val
        self.refresh()

    def load_lang(self,lang):
        path = os.path.join(os.path.dirname(__file__), str(lang) + ".csv")
        if not os.path.exists(path):
            messagebox.showerror("Erreur", f"Le fichier {lang}.csv n'existe pas.")
            self.lang_data = {key: "" for key in self.en_data}
            with open(path, 'w', encoding='utf-8', newline='') as f:
                w = csv.writer(f, delimiter=':')
                for key in self.en_data:
                    w.writerow([key, ""])
        if not path: return
        self.lang_file = path; self.lang_data.clear()
        # Charger toutes les clés/valeurs existantes
        with open(path, encoding='utf-8', newline='') as f:
            for row in csv.reader(f, delimiter=':'):
                if len(row) == 2:
                    key, val = row
                    self.lang_data[key] = val

        hasMissingKey = False
        # Ajouter uniquement les clés manquantes sans écraser les autres
        for key in self.en_data:
            if key not in self.lang_data:
                self.lang_data[key] = ""
                hasMissingKey = True
        if hasMissingKey:
            # Sauvegarder toutes les clés (anciennes + nouvelles)
            with open(path, 'w', encoding='utf-8', newline='') as f:
                w = csv.writer(f, delimiter=':')
                for key in self.en_data:
                    w.writerow([key, self.lang_data.get(key, "")])
            messagebox.showinfo("Succès", "Le fichier viens d'etre corriger (manque de key).")

        self.refresh()

    def refresh(self):
        for w in self.scrollable.winfo_children(): w.destroy()
        self.entries.clear()
        if not self.en_data or not self.lang_file: return
        missing = [k for k in self.en_data if k not in self.lang_data or not self.lang_data[k].strip()]
        self.missing_label.config(text="Missing keys: " + ", ".join(missing)) if missing else self.missing_label.config(text="")

        # en-têtes
        tk.Label(self.scrollable, text="Key", width=50, anchor='w', font=('Arial',10,'bold')).grid(row=0,column=0,padx=5,pady=2)
        tk.Label(self.scrollable, text="en",  width=50, anchor='w', font=('Arial',10,'bold')).grid(row=0,column=1,padx=5,pady=2)
        tk.Label(self.scrollable, text="lang",width=50, anchor='w', font=('Arial',10,'bold')).grid(row=0,column=2,padx=5,pady=2)

        for i,(key,val) in enumerate(self.en_data.items(), start=1):
            tk.Label(self.scrollable, text=key, width=50, anchor='w').grid(row=i,column=0, sticky='w', padx=5)
            tk.Label(self.scrollable, text=val, width=50, anchor='w').grid(row=i,column=1, sticky='w', padx=5)
            var = tk.StringVar(value=self.lang_data.get(key, ""))
            ent = tk.Entry(self.scrollable, textvariable=var, width=50)
            ent.grid(row=i, column=2, pady=1, padx=5)
            if key in missing: ent.config(bg='pink')
            self.entries[key] = var

    def save_lang(self):
        if not self.lang_file:
            messagebox.showwarning("Attention", "Chargez d'abord un fichier de langue")
            return
        with open(self.lang_file, 'w', encoding='utf-8', newline='') as f:
            w = csv.writer(f, delimiter=':')  # Utilisation de ':' comme délimiteur
            for key in self.en_data:
                w.writerow([key, self.entries[key].get()])
        messagebox.showinfo("Succès", "Fichier enregistré.")
        self.load_lang(self.language_var.get())
        self.refresh()

if __name__ == '__main__':
    root = tk.Tk()
    TranslationEditor(root)

    root.mainloop()

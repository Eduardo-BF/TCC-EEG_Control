## Para configurar o BLE:

No ESP-IDF Terminal:

```idf.py menuconfig```

Navegue até as Configurações de Bluetooth
Use as setas para navegar pelo menu seguindo o caminho:
```Component config --->   Bluetooth --->```

Ative o Bluetooth
Certifique-se de que a opção ```[*] Bluetooth``` esteja marcada. Se não estiver, pressione ```Y``` ou a barra de Espaço para ativá-la.

Selecione a Pilha (Stack) NimBLE
Dentro do menu Bluetooth, entre em  ```Host --->``` e selecione a opção ```(X) NimBLE - BLE only```.

Salve e Saia
Pressione a tecla ```S``` para salvar a configuração e, em seguida, ```Q``` para sair do *menuconfig*.

Recompile o Projeto
Para que as novas configurações sejam aplicadas, é necessário recompilar o projeto:


```idf.py build```


##### Resumo: 

``idf.py menuconfig``
```
Component config  ---> 
└── Bluetooth  ---> 
      ├── [*] (Ativar) 
      └── Host ---> 
           └── (X)NimBLE - BLE only
```

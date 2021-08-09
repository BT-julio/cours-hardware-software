#commande d'un moteur DC bidirectionnel via PWM

ce projet qui entre dans le cadre du cours de hardware-software à pour but de piloter à partir d'un software une carte FPGA afin de commander un Moteur Dc bidirectionnel.
pour se faire, nous avons recupérer un driver Pwm sur un github (https://github.com/Jolan-Dubrulle/Driver-PWM-bidirectional-DC-Motor). ce driver écris en langage vhdl, nous avons adapté ce code sur notre carte DE0-nano-SoC d'altéra via l'interface que nous propose Quartus (Quartus Prime 18.1) Lite Edition afin de piloter notre moteur.

Dans ce projet, nous avons écris un code en langage C qui sert à piloter les boutons sur la carte. ces boutons servent ici à augmenter ou reduire la largeur du PWM ce qui engendre ici la variation de vitesse de plus changer le sens de rotation du moteur.

Vous trouverez une explication plus détaillée des différentes étapes réalisé pour mener à bien ce projet dans le fichier hardwaresoftware tutauriel, ainsi que les codes commentés dans le fichiers projet_HW.zip. 

temperatura = int ( input ( "cual es la temperatura: "  ) )

if temperatura <= 5:
    print("no salgas")
elif temperatura <= 12:
    print( "abrigate" )
elif temperatura <= 25:
    print( "anda nomas" )
else:
    print( "ponete protectore" )



entrada = input ( "Teneas una entrada??: " )
edad = int ( input ( "Que edad tenes??? : " ) )

if edad > 18 and entrada.lower() == "si":
    print( "entra" )
else:
    print ( "nono, chau chau" )

numero = int (input ("Decime un numero del 1 al 10 : ") )

for i in range(0, 11):
    print(numero * 2)
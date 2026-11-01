#!/usr/bin/env python3
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
checks=0
for name,mask in [('salao_principal','colisoes_salao'),('banheiro','colisoes_banheiro'),('escritorio','colisoes_escritorio'),('estoque','colisoes_estoque'),('sala_projecao','colisoes_sala_projecao')]:
    image=Image.open(root/f'assets/cenarios/{name}.png');image.load()
    m=Image.open(root/f'assets/cenarios/{mask}.png');m.load()
    assert image.size==m.size==(1672,941),(name,image.size,m.size)
    assert m.mode=='RGBA', (mask,m.mode)
    colors=m.getcolors(m.width*m.height)
    assert all(a==0 or (r,g,b,a)==(255,255,255,255) for count,(r,g,b,a) in colors),mask
    alpha=m.getchannel('A').histogram()
    assert alpha[0]>1_000_000 and alpha[255]>1000 and sum(alpha[1:255])==0
    checks+=4
    print(f'MASK {name}: 1672x941, alpha binário, {alpha[255]} pixels bloqueados')
for file in (root/'assets').rglob('*.png'):
    with Image.open(file) as im:im.verify()
    checks+=1
# All four directions must have one idle and four walk frames with visible pixels.
for direction in ('baixo','cima','direita','esquerda'):
    folder=root/'assets/personagens/john'/direction
    names=[f'john_parado_{direction}.png']+[f'john_andando_{direction}{i}.png' for i in range(1,5)]
    for name in names:
        with Image.open(folder/name) as im:
            assert im.convert('RGBA').getchannel('A').getbbox(),name
        checks+=1
print(f'ASSETS: {checks} verificações aprovadas')

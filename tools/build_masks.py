#!/usr/bin/env python3
"""Máscaras geométricas, sem antialiasing; nunca modifica o banheiro ou os cenários.
Requer Pillow somente para regeneração/inspeção, não durante a execução do jogo.
"""
import json
from pathlib import Path
from PIL import Image, ImageDraw
ROOT = Path(__file__).resolve().parents[1]
data = json.loads((ROOT / 'tools/masks.json').read_text())
for room, geometry in data['rooms'].items():
    mask = Image.new('RGBA', tuple(data['size']), (0, 0, 0, 0))
    draw = ImageDraw.Draw(mask)
    for polygon in [geometry['boundary'], *geometry['obstacles']]:
        points = [tuple(p) for p in polygon]
        draw.line(points + points[:1], fill=(255, 255, 255, 255), width=data['line_width'], joint='curve')
    path = ROOT / f'assets/cenarios/colisoes_{room}.png'
    mask.save(path)
    print(path.relative_to(ROOT))

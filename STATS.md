# Statistics

## Commands

### Installation

	pip3 install labours
	curl -s -L https://github.com/src-d/hercules/releases/download/v10.7.2/hercules.linux_amd64.gz > hercules.gz
	gzip -d hercules.gz
	chmod +x hercules

### Generate the Graphs Below

	hercules --burndown https://github.com/notcompsky/tagem | labours -m burndown-project --resample month -o /tmp/monthly-burndown.png
	hercules --devs https://github.com/notcompsky/tagem | labours -m old-vs-new -o /tmp/lines-changed_old-vs-new.png
	hercules --devs https://github.com/notcompsky/tagem | labours -m devs -o /tmp/devs.png

### Misc Graphs of Interest

	hercules --burndown --languages C,C++ ~/repos/compsky/tagem | labours -m burndown-project --resample month
	hercules --burndown --languages JavaScript ~/repos/compsky/tagem | labours -m burndown-project --resample month
	hercules --burndown --languages CSS ~/repos/compsky/tagem | labours -m burndown-project --resample month
	hercules --burndown --languages Python ~/repos/compsky/tagem | labours -m burndown-project --resample month

## Graphs

![lines-changed_old-vs-new](https://user-images.githubusercontent.com/30552567/89189604-1dcde700-d598-11ea-9960-4d0f6977f947.png)
![burndown](https://user-images.githubusercontent.com/30552567/89189627-2aead600-d598-11ea-90de-9c550d77821c.png)

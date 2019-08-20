
syllabus.pdf: README.md
	pandoc --metadata=linkcolor:blue -t latex -o syllabus.pdf README.md

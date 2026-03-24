$proj = "C:\Users\Daniel\Desktop\MY CLAUDE PROJECTS\TB303Clone"
Set-Location $proj
git config user.email "daniel@tb303clone.local"
git config user.name "Daniel"
git add Source/ Scripts/ CMakeLists.txt README_BUILD.md
git commit -m "CHECKPOINT v1.0 - TB303 Clone complete: 16 presets, accent scream, diode ladder filter, delay note divisions"
Write-Host "Checkpoint created!"

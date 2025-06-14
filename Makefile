.PHONY: generatenb
generatenb:
	jupyter nbconvert scanner3dV2/notebooks/main_pipeline.ipynb \
		--to html \
		--output-dir=public/v2nb \
		--TemplateExporter.extra_template_basedirs=scanner3dV2/notebooks/ \
		--template mytemplate \
		--no-input \
		--execute
	cp -r scanner3dV2/notebooks/mytemplate/static public/v2nb
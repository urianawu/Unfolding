#include "UI/ConnectorPanel.h"

QFont ConnectorPanel::fontfamily = QFont("Arial");
Float ConnectorPanel::fontSize = 20.0f;

ConnectorPanel::ConnectorPanel(int procType)
	: ui(new Ui::ConnectorPanel)
{
	ui->setupUi(this);

	resetParas(procType);

	connect(ui->file_button, &QAbstractButton::clicked, this, &ConnectorPanel::setFileName);
	connect(ui->save_button, &QDialogButtonBox::clicked, this, &QDialog::close);
	connect(ui->save_button, &QDialogButtonBox::clicked, this, &ConnectorPanel::save);

	connect(ui->etchseg_val,
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		[&](int value) {
		bool res = value == 0;
		ui->etchsegwid_label->setDisabled(res);
		ui->etchsegwid_val->setDisabled(res);
		ui->etchsegwid_unit->setDisabled(res);
	});
}

QString ConnectorPanel::getFilename() const
{
	return ui->filename_text->text();
}



void ConnectorPanel::resetParas(int procType)
{
	//meshType = procType;
	//ui->mesh_type->setCurrentIndex(procType);
	ui->connector_type->clear();
	switch (procType)
	{
	case HDS_Mesh::HALFEDGE_PROC:
		ui->mesh_type_val->setText("Half-Edge Mesh");
		ui->connector_type->addItem("Simple");
		ui->connector_type->addItem("Insert");
		ui->connector_type->addItem("Gear");
		ui->connector_type->addItem("Saw");
		ui->connector_type->addItem("Advanced Saw");
		break;
	case HDS_Mesh::QUAD_PROC:
		ui->mesh_type_val->setText("Quad-Edge Mesh");

		ui->pinholeunit_type->setDisabled(false);
		ui->pinholesize_label->setDisabled(false);
		ui->pinholesize_val->setDisabled(false);
		ui->connector_type->addItem("Holes");
		ui->connector_type->addItem("Faces");
		break;
	case HDS_Mesh::WINGED_PROC:
		ui->mesh_type_val->setText("Winged-Edge Mesh");

		ui->pinholeunit_type->setDisabled(false);
		ui->pinholesize_label->setDisabled(false);
		ui->pinholesize_val->setDisabled(false);
		ui->connector_type->addItem("Holes");
		ui->connector_type->addItem("Faces");
		break;
	case HDS_Mesh::GRS_PROC:
		ui->mesh_type_val->setText("Graph Rotation System Mesh");
		break;
	case HDS_Mesh::FBWALK_PROC:
		ui->mesh_type_val->setText("Face Boundary Walk Mesh");
		ui->connector_type->addItem("Arch");
		ui->connector_type->addItem("Ring");
		break;
		// TODO: add woven
	case HDS_Mesh::WOVEN_PROC:
		ui->mesh_type_val->setText("Woven Mesh");
		ui->connector_type->addItem("Holes");
		break;
	default:
		break;
	}
}

void ConnectorPanel::setFileName()
{
	QString filename = QFileDialog::getSaveFileName(this, "Export file as", "", tr("SVG files (*.svg)"));
	if (!filename.isEmpty())
	{
		ui->filename_text->setText(filename);
	}
}

void ConnectorPanel::save()
{
	fontfamily = ui->font_val->currentFont();
	fontSize = ui->font_size_val->value();

	conf["connector"] = static_cast<Float>(ui->connector_type->currentIndex());
	// Material Connections
	conf["matConnLen"] = ui->matconn_len_val->value();
	conf["matConnUnit"] = static_cast<Float>(ui->matconn_unit_type->currentIndex());

	// Export Graph Stroke Width
	conf["strokeWd"] = ui->stroke_wd_val->value();
	// Graph Scaling and Boundary Size
	conf["scale"] = ui->scale_val->value();
	conf["width"] = ui->width_val->value();
	conf["length"] = ui->length_val->value();

	// Pinhole Utilities
	conf["pinUnit"] = static_cast<Float>(ui->pinholeunit_type->currentIndex());
	conf["pinSize"] = ui->pinholesize_val->value();
	conf["pinCount"] = static_cast<Float>(ui->pinholecount_type->currentIndex());

	// Etch Utilities
	conf["etchSeg"] = (double)ui->etchseg_val->value();
	conf["etchSegWidth"] = ui->etchsegwid_val->value();
	conf["scoreType"] = (double)ui->score_type->currentIndex();
	conf["dashLen"] = ui->scoredash_len->value();
	conf["dashGap"] = ui->scoredash_gap->value();
	conf["dashUnit"] = static_cast<Float>(ui->scoredash_unit->currentIndex());

	emit sig_saved();
}
